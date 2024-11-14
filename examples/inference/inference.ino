#include <SSCMA_Micro_Core.h>

#include <Arduino.h>
#include <esp_camera.h>


SET_LOOP_TASK_STACK_SIZE(40 * 1024);


SSCMAMicroCore instance;
SSCMAMicroCore::VideoCapture capture;


void setup() {

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // Init serial port
    Serial.begin(115200);

    // Init video capture
    MA_RETURN_IF_UNEXPECTED(capture.begin(SSCMAMicroCore::VideoCapture::DefaultCameraConfigXIAOS3));

    // Init SSCMA Micro Core
    MA_RETURN_IF_UNEXPECTED(instance.begin(SSCMAMicroCore::Config::DefaultConfig));

    Serial.println("Init done");

}

void loop() {

    auto frame = capture.getManagedFrame();

    MA_RETURN_IF_UNEXPECTED(instance.invoke(frame));

    auto boxes = instance.getBoxes();
    for (const auto& box : boxes) {
        Serial.printf("Box: x=%f, y=%f, w=%f, h=%f, score=%f, target=%d\n", box.x, box.y, box.w, box.h, box.score, box.target);
    }

    auto classes = instance.getClasses();
    for (const auto& cls : classes) {
        Serial.printf("Class: target=%d, score=%f\n", cls.target, cls.score);
    }

    auto points = instance.getPoints();
    for (const auto& point : points) {
        Serial.printf("Point: x=%f, y=%f, z=%f, score=%f, target=%d\n", point.x, point.y, point.z, point.score, point.target);
    }

    auto keypoints = instance.getKeypoints();
    for (const auto& kp : keypoints) {
        Serial.printf("Keypoints: box: x=%f, y=%f, w=%f, h=%f, score=%f, target=%d\n", kp.box.x, kp.box.y, kp.box.w, kp.box.h, kp.box.score, kp.box.target);
        for (const auto& point : kp.points) {
            Serial.printf("Keypoint: x=%f, y=%f, z=%f, score=%f, target=%d\n", point.x, point.y, point.z, point.score, point.target);
        }
    }

    auto perf = instance.getPerf();
    Serial.printf("Perf: preprocess=%dms, inference=%dms, postprocess=%dms\n", perf.preprocess, perf.inference, perf.postprocess);

}