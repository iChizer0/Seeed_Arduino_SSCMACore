/***
 * @file SSCMA_Micro_Core.h
 *
 * @details This header file declares the SSCMA Micro Core class, which
 * provides a microcontroller optimized AI inference API for Arduino.
 *
 * Copyright (C) 2024Seeed Technology Co., Ltd. All right reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#pragma once

#ifndef _SSCMA_MICRO_CORE_H_
#define _SSCMA_MICRO_CORE_H_


#if defined(__AVR__)
#error "Insufficient memory: This code cannot run on any AVR platform due to limited memory."
#endif


#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <functional>
#include <memory>
#include <string>
#include <vector>


#if (CONFIG_IDF_TARGET_ESP32 | CONFIG_IDF_TARGET_ESP32S3 | CONFIG_IDF_TARGET_ESP32S2)
#define MA_PORTING_ESPRESSIF_ESP32 1
#endif


#if MA_PORTING_ESPRESSIF_ESP32
#include <esp_camera.h>
#endif


/**
 * @brief The SSCMAMicroCore class provides a microcontroller optimized for SSCMA.
 */
class SSCMAMicroCore {
public:
    /**
     * @brief Configuration for invoking the SSCMAMicroCore.
     */
    struct InvokeConfig {
        int top_k;              ///< The number of top results to return.
        float score_threshold;  ///< The minimum score threshold for results.
        float nms_threshold;    ///< The non-maximum suppression threshold.
    };

    /**
     * @brief Configuration for the SSCMAMicroCore.
     */
    struct Config {
        int model_id;                       ///< The ID of the model to use.
        int algorithm_id;                   ///< The ID of the algorithm to use.
        const InvokeConfig* invoke_config;  ///< The invocation configuration.
    };

    /**
     * @brief Represents a bounding box in an image.
     */
    struct Box {
        float x;      ///< The x-coordinate of the box.
        float y;      ///< The y-coordinate of the box.
        float w;      ///< The width of the box.
        float h;      ///< The height of the box.
        float score;  ///< The confidence score of the box.
        int target;   ///< The target ID associated with the box.
    };

    /**
     * @brief Represents a class detection result.
     */
    struct Class {
        int target;   ///< The target ID of the class.
        float score;  ///< The confidence score of the class.
    };

    /**
     * @brief Represents a point in 3D / 2D (with score) space.
     */
    struct Point {
        float x;      ///< The x-coordinate of the point.
        float y;      ///< The y-coordinate of the point.
        float z;      ///< The z-coordinate of the point.
        float score;  ///< The confidence score of the point.
        int target;   ///< The target ID associated with the point.
    };

    /**
     * @brief Represents keypoints detected in an image.
     */
    struct Keypoints {
        Box box;                    ///< The bounding box of the keypoints.
        std::vector<Point> points;  ///< The list of keypoints.
    };

    /**
     * @brief Typedef for a callback function that handles boxes.
     */
    using BoxesCallback = std::function<void(const std::vector<Box>&, void*)>;

    /**
     * @brief Typedef for a callback function that handles classes.
     */
    using ClassesCallback = std::function<void(const std::vector<Class>&, void*)>;

    /**
     * @brief Typedef for a callback function that handles points.
     */
    using PointsCallback = std::function<void(const std::vector<Point>&, void*)>;

    /**
     * @brief Typedef for a callback function that handles keypoints.
     */
    using KeypointsCallback = std::function<void(const std::vector<Keypoints>&, void*)>;

    /**
     * @brief Performance metrics for the SSCMAMicroCore.
     */
    struct Perf {
        uint32_t preprocess;   ///< Time taken for preprocessing.
        uint32_t inference;    ///< Time taken for inference.
        uint32_t postprocess;  ///< Time taken for postprocessing.
    };

    /**
     * @brief Typedef for a callback function that handles performance metrics.
     */
    using PerfCallback = std::function<void(const Perf&, void*)>;

    /**
     * @brief Enum for pixel formats.
     */
    enum PixelFormat {
        kUNKNOWN = 0,  ///< Unknown pixel format.
        kRGB888  = 1,  ///< 24-bit RGB pixel format.
        kRGB565,       ///< 16-bit RGB pixel format.
        kGRAY8,        ///< 8-bit grayscale pixel format.
        kJPEG          ///< JPEG compressed image format.
    };

    /**
     * @brief Represents a frame of image data.
     */
    struct Frame {
        PixelFormat format;        ///< The pixel format of the frame.
        uint16_t width;            ///< The width of the frame.
        uint16_t height;           ///< The height of the frame.
        uint16_t orientation;      ///< The orientation of the frame.
        struct timeval timestamp;  ///< The timestamp of the frame.
        uint32_t size;             ///< The size of the frame data.
        uint8_t* data;             ///< The frame data.

#if MA_PORTING_ESPRESSIF_ESP32
        /**
         * @brief Creates a Frame from an ESP32 camera frame.
         * @param frame The ESP32 camera frame to convert from.
         * @return A new Frame instance.
         */
        static Frame fromCameraFrame(const camera_fb_t* frame);
#endif
    };

    /**
     * @brief Represents an expected result with a success flag and message.
     */
    struct Expected {
        bool success;         ///< Whether the operation was successful.
        std::string message;  ///< A message describing the result.
    };

public:
    /**
     * @brief Constructs a new SSCMAMicroCore instance.
     */
    SSCMAMicroCore();

    /**
     * @brief Destructs the SSCMAMicroCore instance.
     */
    ~SSCMAMicroCore();

    /**
     * @brief Begins the SSCMAMicroCore with the given configuration.
     * @param config The configuration to use.
     * @return An Expected result indicating success or failure.
     */
    Expected begin(const Config& config);

    /**
     * @brief Invokes the SSCMAMicroCore with the given frame and optional configuration.
     * @param frame The frame to process.
     * @param config The optional invocation configuration (overrides the current configuration).
     * @param user_context User-provided context to pass to callbacks.
     * @return An Expected result indicating success or failure.
     */
    Expected invoke(const Frame& frame, const InvokeConfig* config = nullptr, void* user_context = nullptr);

    /**
     * @brief Registers a callback for box detection results.
     * @param callback The callback function to register.
     */
    void registerBoxesCallback(BoxesCallback callback);

    /**
     * @brief Registers a callback for class detection results.
     * @param callback The callback function to register.
     */
    void registerClassesCallback(ClassesCallback callback);

    /**
     * @brief Registers a callback for point detection results.
     * @param callback The callback function to register.
     */
    void registerPointsCallback(PointsCallback callback);

    /**
     * @brief Registers a callback for keypoint detection results.
     * @param callback The callback function to register.
     */
    void registerKeypointsCallback(KeypointsCallback callback);

    /**
     * @brief Registers a callback for performance metrics.
     * @param callback The callback function to register.
     */
    void registerPerfCallback(PerfCallback callback);

private:
    bool _initialized;  ///< Whether the SSCMAMicroCore is initialized.
    Config _config;     ///< The current configuration of the SSCMAMicroCore.

    BoxesCallback _boxes_callback;          ///< The registered box callback.
    ClassesCallback _classes_callback;      ///< The registered class callback.
    PointsCallback _points_callback;        ///< The registered point callback.
    KeypointsCallback _keypoints_callback;  ///< The registered keypoint callback.

    PerfCallback _perf_callback;  ///< The registered performance callback.
};

#endif  //_SSCMA_MICRO_CORE_H_
