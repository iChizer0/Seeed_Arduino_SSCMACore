# SSCMA Micro Core for Arduino

SSCMA Micro Core for Arduino is a library designed for Arduino, which supports a variety of AI algorithms (Classification, Detection, Pose Estimation, etc.). The library provides a simple and easy-to-use interface for developers to seamlessly integrate SSCMA-Micro Core with their Arduino projects.

## Installation

1. Download the latest version of the library from our [GitHub repository](https://github.com/seeed-studio/Seeed_Arduino_SSCMACore).
2. Add the library to your Arduino IDE by selecting **Sketch > Include Library > Add .ZIP Library** and choosing the downloaded file.

## Usage

1. Use [SenseCreaft AI Platform](https://sensecraft.seeed.cc/ai/#/home) or other tools to flash a compatiable model to your device.

2. Include the library in your Arduino sketch:  
   ```c++
   #include <SSCMA_Micro_Core.h>
   ```

3. Create an instance of the SSCMAMicroCore:
   ```c++
   SSCMAMicroCore instance;
   ```

4. Initialize the SSCMAMicroCore and register callback functions in `setup()` function:
    ```c++
    void setup() {
        // Initialize Serial, Camera, etc.

        instance.begin(SSCMAMicroCore::Config{});

        instance.registerBoxesCallback([](const std::vector<SSCMAMicroCore::Box>& boxes, void* user_context) {
            Serial.printf("Boxes: %d\n", boxes.size());
            for (const auto& box : boxes) {
                Serial.printf("Box: %f %f %f %f %f %d\n", box.x, box.y, box.w, box.h, box.score, box.target);
            }
        });
    }
    ```

4. Call the `invoke()` method in the `loop()` function to process the sensor data, then the registered callback function will be called (if the output category is registered in corresponding callback functions):
    ```c++
    void loop() {
        camera_fb_t* frame = esp_camera_fb_get();
        if (frame == nullptr) {
            Serial.println("Camera capture failed");
            return;
        }
    
        auto ret = instance.invoke(SSCMAMicroCore::Frame::fromCameraFrame(frame));
        if (!ret.success) {
            Serial.println(ret.message.c_str());
        }
    
        esp_camera_fb_return(frame);
    }
    ```

## Example

Please visit the [examples](./examples) folder to find sample sketches that demonstrate how to use the library with supported sensors.

## API Reference

### Constructors & Destructors

- `SSCMAMicroCore()`
  - Initializes a new instance of the `SSCMAMicroCore` class. This is where any necessary setup or initialization can occur.

- `~SSCMAMicroCore()`
  - Cleans up any resources allocated by the `SSCMAMicroCore` instance. This is where any necessary cleanup or deinitialization should occur.

### Configuration and Invocation

- `begin(const Config& config)`
  - This method initializes the `SSCMAMicroCore` with a specific configuration. It sets up the model and algorithm to be used for subsequent detections.
  - **Returns:** An `Expected` struct containing a boolean `success` indicating whether the operation was successful and a `message` providing additional information.

- `invoke(const Frame& frame, const InvokeConfig* config = nullptr, void* user_context = nullptr)`
  - This method processes a single frame using the `SSCMAMicroCore`. It can be called repeatedly to process multiple frames.
  - **Parameters:**
    - `frame`: The frame of image data to be processed.
    - `config`: An optional pointer to an `InvokeConfig` struct that overrides the configuration set in `begin`.
    - `user_context`: A user-provided pointer that will be passed to the registered callbacks.
  - **Returns:** An `Expected` struct containing a boolean `success` indicating whether the operation was successful and a `message` providing additional information.

### Callback Registration

- `void registerBoxesCallback(BoxesCallback callback)`
  - Registers a callback function that will be called when box detection results are available.

- `void registerClassesCallback(ClassesCallback callback)`
  - Registers a callback function that will be called when class detection results are available.

- `void registerPointsCallback(PointsCallback callback)`
  - Registers a callback function that will be called when point detection results are available.

- `void registerKeypointsCallback(KeypointsCallback callback)`
  - Registers a callback function that will be called when keypoint detection results are available.

- `void registerPerfCallback(PerfCallback callback)`
  - Registers a callback function that will be called when performance metrics are available.

Please refer to the [header files](./src/SSCMA_Micro_Core.h) for detailed information about the library's types and APIs.

## Compatibility

This library is compatible with Arduino boards and sensors that support the SSCMA-Micro firmware.

## License

This library is released under the [Apache License](./LICENSE).

## Contributions

Contributions and improvements to the library are welcomed. If you encounter any issues or have suggestions for enhancements, please create an issue or pull request [here](https://github.com/your_repository_link).

For more detailed information, including specific API references, please refer to the header files and the provided examples included in the library.

Thank you for using the Seeed_Arduino_SSCMACore library! We hope it enhances your experience in working with SSCMA-Micro Core with supported sensors on the Arduino platform.
