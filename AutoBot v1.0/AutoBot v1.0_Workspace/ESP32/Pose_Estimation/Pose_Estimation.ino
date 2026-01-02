#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
#include "esp_camera.h"

#include "apriltag.h"
#include "tag16h5.h"
#include "common/image_u8.h"
#include "common/zarray.h"
#include "apriltag_pose.h"
#include "common/matd.h"

#define TAG_SIZE 0.05
#define FX 924.71
#define FY 924.71
#define CX 403.80
#define CY 305.08
#define LED_PIN 4

apriltag_family_t *tf;
apriltag_detector_t *td;

bool startDetection = false;  // flag to start camera operation
String inputString = "";      // buffer for serial input

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    psramInit();

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_GRAYSCALE;
    config.frame_size = FRAMESIZE_QVGA;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;
    config.fb_count = 1;

    if (esp_camera_init(&config) != ESP_OK) {
        Serial.println("Camera init failed");
        ESP.restart();
    }

    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);

    tf = tag16h5_create();
    td = apriltag_detector_create();
    apriltag_detector_add_family_bits(td, tf, 1);
    td->quad_decimate = 1.0;
    td->quad_sigma = 0.0;
    td->refine_edges = 1;
    td->decode_sharpening = 0.25;
    td->nthreads = 2;
    td->debug = 0;

    Serial.println("System Ready. Send 'YES' to start AprilTag detection.");
}

void loop() {
    // --- Check for Serial input trigger ---
    if (Serial.available()) {
        inputString = Serial.readStringUntil('\n');
        inputString.trim();  // remove newline/spaces

        if (inputString.equalsIgnoreCase("YES")) {
            startDetection = true;
            Serial.println("Detection Started...");
            digitalWrite(LED_PIN, HIGH);
            delay(200);
            digitalWrite(LED_PIN, LOW);
        } else if (inputString.equalsIgnoreCase("STOP")) {
            startDetection = false;
            Serial.println("Detection Stopped.");
            digitalWrite(LED_PIN, LOW);
        }
    }

    // --- Only run detection when 'YES' received ---
    if (!startDetection) return;

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) return;

    image_u8_t im = {fb->width, fb->height, fb->width, fb->buf};
    zarray_t *detections = apriltag_detector_detect(td, &im);

    if (zarray_size(detections) > 0) {
        static unsigned long lastBlink = 0;
        unsigned long now = millis();
        if (now - lastBlink > 50) {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            lastBlink = now;
        }

        for (int i = 0; i < zarray_size(detections); i++) {
            apriltag_detection_t *det;
            zarray_get(detections, i, &det);

            Serial.printf("Tag ID: %d\n", det->id);

            apriltag_detection_info_t info = {det, TAG_SIZE, FX, FY, CX, CY};
            apriltag_pose_t pose;
            estimate_tag_pose(&info, &pose);

            double yaw = atan2(MATD_EL(pose.R, 1, 0), MATD_EL(pose.R, 0, 0)) * 180 / M_PI;
            double pitch = atan2(-MATD_EL(pose.R, 2, 0),
                                 sqrt(pow(MATD_EL(pose.R, 2, 1), 2) + pow(MATD_EL(pose.R, 2, 2), 2))) * 180 / M_PI;
            double roll = atan2(MATD_EL(pose.R, 2, 1), MATD_EL(pose.R, 2, 2)) * 180 / M_PI;

            Serial.printf("Yaw: %.2f Pitch: %.2f Roll: %.2f\n", yaw, pitch, roll);

            matd_t *R_transpose = matd_transpose(pose.R);
            for (int j = 0; j < pose.t->nrows; j++) {
                MATD_EL(pose.t, j, 0) = -MATD_EL(pose.t, j, 0);
            }
            matd_t *cam_pos = matd_multiply(R_transpose, pose.t);
            Serial.printf("X: %.2f Y: %.2f Z: %.2f\n", MATD_EL(cam_pos, 0, 0), MATD_EL(cam_pos, 1, 0), MATD_EL(cam_pos, 2, 0));
            matd_destroy(R_transpose);
            matd_destroy(cam_pos);
        }
    } else {
        digitalWrite(LED_PIN, LOW);
    }

    apriltag_detections_destroy(detections);
    esp_camera_fb_return(fb);
}