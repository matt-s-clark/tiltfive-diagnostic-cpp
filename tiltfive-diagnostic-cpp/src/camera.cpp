/*
 * Copyright (C) 2020-2023 Tilt Five, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 /// \file
 /// \brief Sample code using the Tilt Five™ C++ API Binder

 /// \privatesection

#include "include/TiltFiveNative.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <chrono>
#include <iostream>
#include <fstream>
#include <map>

using namespace cv;

/// \private
using Client = std::shared_ptr<tiltfive::Client>;
/// \private
using Glasses = std::shared_ptr<tiltfive::Glasses>;
/// \private
using Wand = std::shared_ptr<tiltfive::Wand>;

// Shim C++14 chrono_literals ms
constexpr std::chrono::milliseconds operator""_ms(unsigned long long ms)
{
    return std::chrono::milliseconds(ms);
}

/// Find the first pair of available glasses
//
/// \param[in] client - std::unique_ptr to a ::Client
/// \return ::Glasses representing the first found glasses or an error
/// [WaitForGlasses]
auto waitForGlasses(Client& client) -> tiltfive::Result<Glasses>
{
    std::cout << "Looking for glasses..." << std::flush;

    // Loop until we find glasses
    auto glassesList = client->listGlasses();
    if (!glassesList)
    {
        return glassesList.error();
    }
    while (glassesList->empty())
    {
        std::cout << "." << std::flush;
        std::this_thread::sleep_for(100_ms);

        // Request a list of the available glasses
        glassesList = client->listGlasses();
        if (!glassesList)
        {
            return glassesList.error();
        }
    }

    // Print out the found glasses
    for (auto& glassesInstance : *glassesList)
    {
        std::cout << "Found : " << glassesInstance << std::endl;
    }

    // Return the first found glasses
    return tiltfive::obtainGlasses(glassesList->front(), client);
}
/// [WaitForGlasses]

auto doThingsWithWands(const Wand& wand) -> tiltfive::Result<void>
{
    std::cout << "Doing something with wand : " << wand << std::endl;

    auto start = std::chrono::steady_clock::now();
    do
    {
        auto report = wand->getLatestReport();
        std::cout << "\r" << report;
    } while ((std::chrono::steady_clock::now() - start) < 10000_ms);

    std::cout << std::endl
        << "Done with wand" << std::endl;

    return tiltfive::kSuccess;
}

auto waitForWand(Glasses& glasses) -> tiltfive::Result<Wand>
{
    std::cout << "Looking for wand..." << std::flush;

    auto wandHelper = glasses->getWandStreamHelper();

    for (;;)
    {
        auto wands = wandHelper->listWands();
        if (wands)
        {
            if (!wands->empty())
            {
                std::cout << "Found : " << wands->front() << std::endl;
                return wands->front();
            }
        }
        else
        {
            std::cerr << "Error listing wands : " << wands << std::endl;
            return wands.error();
        }

        std::cout << "." << std::flush;
        std::this_thread::sleep_for(100_ms);
    }
}

auto initCameraImage(Glasses& glasses, T5_CamImage* imageBuffer) -> tiltfive::Result<void>
{
    imageBuffer->bufferSize = T5_MIN_CAM_IMAGE_BUFFER_WIDTH * T5_MIN_CAM_IMAGE_BUFFER_HEIGHT;
    imageBuffer->cameraIndex = 0;
    imageBuffer->pixelData = new uint8_t[T5_MIN_CAM_IMAGE_BUFFER_WIDTH * T5_MIN_CAM_IMAGE_BUFFER_HEIGHT];

    auto result = glasses->submitEmptyCamImageBuffer(imageBuffer);

    std::cout << "\nResult:       " << result << "\n";
    std::cout << "Buffer: " << imageBuffer << "\n\n";

    return result;
}

static std::string roundNum(float num)
{
    double value = std::round(num * 1000.0) / 1000.0;
    std::string num_text = std::to_string(value);

    return num_text.substr(0, num_text.find(".") + 4);
}

/// [ExclusiveOps]
auto readPoses(Glasses& glasses) -> tiltfive::Result<void>
{
    auto readyResult = glasses->ensureReady();
    std::cout << "Glasses Status: " << readyResult << "\n";
    if (readyResult.error().value() != T5_SUCCESS) {
        std::cout << "*** GLASSES UNAVAILABLE\n";
        return readyResult;
    }

    T5_CamImage* camImageBuffer = new T5_CamImage();
    auto submitResult = initCameraImage(glasses, camImageBuffer);

    cv::namedWindow("Test Window", cv::WINDOW_AUTOSIZE);

    int count = 0;
    int successCount = 0;
    std::map<std::error_code, int> errorCodeCount;
    std::map<float, int> xPosDict;

    auto start = std::chrono::steady_clock::now();
    do
    {
        count++;

        auto pose = glasses->getLatestGlassesPose(kT5_GlassesPoseUsage_GlassesPresentation);
        auto imageRead = glasses->getFilledCamImageBuffer();
        errorCodeCount[imageRead.error()]++;

        // posCAM_GBD doesn't seem to work. This code is for debugging.
        auto it = xPosDict.find(camImageBuffer->posCAM_GBD.x);
        if (it != xPosDict.end()) {
            xPosDict[camImageBuffer->posCAM_GBD.x]++;
        }
        else {
            xPosDict[camImageBuffer->posCAM_GBD.x] = 1;
        }

        if (imageRead.error().value() == 0) {
            cv::Mat img(T5_MIN_CAM_IMAGE_BUFFER_HEIGHT, T5_MIN_CAM_IMAGE_BUFFER_WIDTH, CV_8U,
                camImageBuffer->pixelData);

            cv::imshow("Test Window", img);
            int k = cv::waitKey(1);

            if (!pose)
            {
                std::cout << "\rImage Success " << successCount << " times out of " << count << " passes - err, err, err - err, err, err, err";
            }
            else
            {
                std::cout << "\rImage Success " << successCount << " times out of " << count << " passes - "
                    << roundNum(pose->posGLS_GBD.x) << ", "
                    << roundNum(pose->posGLS_GBD.y) << ", "
                    << roundNum(pose->posGLS_GBD.z) << " - "
                    << roundNum(pose->rotToGLS_GBD.x) << ", " 
                    << roundNum(pose->rotToGLS_GBD.y) << ", "
                    << roundNum(pose->rotToGLS_GBD.z) << ", "
                    << roundNum(pose->rotToGLS_GBD.w);
            }


            successCount++;
            if (successCount == 1)
            {
                // Save the Mat as a PNG image
                bool success = cv::imwrite("camera-frame.png", img);

                if (success) {
                    std::cout << "\n\nImage saved successfully as 'camera-frame.png'." << std::endl;
                }
                else {
                    std::cerr << "\n\nError saving the image.\n\n" << std::endl;
                }
            }

            // If you need to spend some time processing the image, you can submit an alternate CamImageBuffer, rather than reuse this one.
            auto resubmitResult = glasses->submitEmptyCamImageBuffer(camImageBuffer);
            if (resubmitResult.error().value() != 0) {
                std::cout << "\n\n** ERROR ON RESET ***\n\n";
            }
        }
       
    } while ((std::chrono::steady_clock::now() - start) < 20000_ms);

    std::cout << "\n\X Positions:\n";
    for (const auto& pair : xPosDict) {
        std::cout << " * Position: " << pair.first << " returned " << pair.second << " times.\n";
    }

    std::cout << "\n\nError Codes:\n";
    for (const auto& pair : errorCodeCount) {
        std::cout << " * Type '" << pair.first << "' returned " << pair.second << " times.\n";
    }

    // TODO: Determine a better way to determine if the image buffer needs to be cleared, successCount seems like a hack
    if (successCount > 0)
    {
        glasses->cancelCamImageBuffer(camImageBuffer->pixelData);
    }

    return tiltfive::kSuccess;
}
/// [ExclusiveOps]

auto doThingsWithGlasses(Glasses& glasses) -> tiltfive::Result<void>
{
    std::cout << "Doing something with : " << glasses << std::endl;

    // Set Config Parameters
    T5_CameraStreamConfig cameraStreamConfig = T5_CameraStreamConfig();
    cameraStreamConfig.cameraIndex = 0;
    cameraStreamConfig.enabled = true;

    glasses->configureCameraStream(cameraStreamConfig);

    /// [NonExclusiveOps]
    // Get the friendly name for the glasses
    // This is the name that's user set in the Tilt Five™ control panel.
    auto friendlyName = glasses->getFriendlyName();
    if (friendlyName)
    {
        std::cout << "Obtained friendly name : " << friendlyName << std::endl;
    }
    else if (friendlyName.error() == tiltfive::Error::kSettingUnknown)
    {
        std::cerr << "Couldn't get friendly name : Service reports it's not set" << std::endl;
    }
    else
    {
        std::cerr << "Error obtaining friendly name : " << friendlyName << std::endl;
    }

    // Get the IPD for the glasses
    // This is user set IPD in the Tilt Five™ control panel.
    auto ipd = glasses->getIpd();
    if (ipd)
    {
        std::cout << "Obtained IPD : " << ipd << "m" << std::endl;
    }
    else if (ipd.error() == tiltfive::Error::kSettingUnknown)
    {
        std::cerr << "Couldn't get IPD : Service reports it's not set" << std::endl;
    }
    else
    {
        std::cerr << "Error obtaining IPD : " << ipd << std::endl;
        return ipd.error();
    }
    /// [NonExclusiveOps]

    {
        /// [Connect]
        // Wait for exclusive glasses connection
        auto connectionHelper = glasses->createConnectionHelper("Awesome game - Player 1");
        auto connectionResult = connectionHelper->awaitConnection(10000_ms);
        if (connectionResult)
        {
            std::cout << "Glasses connected for exclusive use" << std::endl;
        }
        else
        {
            std::cerr << "Error connecting glasses for exclusive use : " << connectionResult
                << std::endl;
            return connectionResult.error();
        }
        /// [Connect]

        // Reading poses
        auto result = readPoses(glasses);
        if (!result)
        {
            std::cerr << "Error reading poses : " << result << std::endl;
            return result.error();
        }
    }

    // The connectionHelper is destroyed at the end of the previous scope, but the glasses are still
    // reserved.  Let's release them, then confirm that exclusive ops such as reading poses fails.
    auto releaseResult = glasses->release();
    if (!releaseResult)
    {
        std::cerr << "Failed to release glasses : " << releaseResult << std::endl;
        return releaseResult.error();
    }
    auto readPosesResult = readPoses(glasses);
    if (readPosesResult)
    {
        std::cerr << "Reading poses unexpectedly succeeded after glasses release\n";
    }
    else if (readPosesResult.error() != tiltfive::Error::kNotConnected)
    {
        std::cerr << "Unexpected pose read error: " << readPosesResult << "\n";
    }

    std::cout << std::endl
        << "Done with glasses" << std::endl;

    return tiltfive::kSuccess;
}

/// [SystemWideQuery]
auto printGameboardDimensions(Client& client) -> tiltfive::Result<void>
{
    auto result = client->getGameboardSize(kT5_GameboardType_LE);
    if (!result)
    {
        return result.error();
    }

    float width = result->viewableExtentPositiveX + result->viewableExtentNegativeX;
    float length = result->viewableExtentPositiveY + result->viewableExtentNegativeY;
    float height = result->viewableExtentPositiveZ;

    std::cout << "LE Gameboard size : " << width << "m x " << length << "m x " << height << "m"
        << std::endl;

    return tiltfive::kSuccess;
}

/// [WaitForServiceCallerFn]
auto printServiceVersion(Client& client) -> tiltfive::Result<void>
{
    auto result = client->getServiceVersion();
    if (!result)
    {
        return result.error();
    }

    std::cout << "Service version : " << result << std::endl;
    return tiltfive::kSuccess;
}
/// [WaitForServiceCallerFn]
/// [SystemWideQuery]

auto printUiStatusFlags(Client& client) -> tiltfive::Result<void>
{
    auto result = client->isTiltFiveUiRequestingAttention();
    if (!result)
    {
        return result.error();
    }
    std::cout << "Tilt Five UI (Attention Requested) : " << ((*result) ? "TRUE" : "FALSE")
        << std::endl;

    return tiltfive::kSuccess;
}

/// [WaitForService]
// Convenience function to repeatedly call another function if it returns 'service unavailable'
template <typename T>
auto waitForService(Client& client, const std::function<tiltfive::Result<T>(Client& client)>& func)
-> tiltfive::Result<T>
{

    bool waitingForService = false;
    for (;;)
    {
        auto result = func(client);
        if (result)
        {
            return result;
        }
        else if (result.error() != tiltfive::Error::kNoService)
        {
            return result.error();
        }

        std::cout << (waitingForService ? "." : "Waiting for service...") << std::flush;
        waitingForService = true;
        std::this_thread::sleep_for(100_ms);
    }
}
/// [WaitForService]

class MyParamChangeListener : public tiltfive::ParamChangeListener
{
    auto onSysParamChanged(const std::vector<T5_ParamSys>& changed) -> void override
    {
        for (const auto& param : changed)
        {
            std::cout << "System Parameter changed : [" << param << "]" << std::endl;
        }
    }

    auto onGlassesParamChanged(const Glasses& glasses, const std::vector<T5_ParamGlasses>& changed)
        -> void override
    {
        for (const auto& param : changed)
        {
            std::cout << "Glasses Parameter changed : " << glasses << " => [" << param << "]"
                << std::endl;
        }
    }
};

int main(int /* argc */, char** /* argv */)
{
    /// [CreateClient]
    // Create the client
    auto client = tiltfive::obtainClient("com.tiltfive.test", "0.1.0", nullptr);
    if (!client)
    {
        std::cerr << "Failed to create client : " << client << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "Obtained client : " << client << std::endl;
    /// [CreateClient]

    // Create a parameter change helper
    std::shared_ptr<MyParamChangeListener> paramChangeListener(new MyParamChangeListener());
    auto paramChangeHelper = (*client)->createParamChangedHelper(paramChangeListener);

    // Get the gameboard dimensions
    auto result = printGameboardDimensions(*client);
    if (!result)
    {
        std::cerr << "Failed to print gameboard dimensions : " << result << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Get the service version
    /// [WaitForServiceCaller]
    result = waitForService<void>(*client, printServiceVersion);
    if (!result)
    {
        std::cerr << "Failed to get service version : " << result << std::endl;
        std::exit(EXIT_FAILURE);
    }
    /// [WaitForServiceCaller]

    // Get the UI flags
    result = waitForService<void>(*client, printUiStatusFlags);
    if (!result)
    {
        std::cerr << "Failed to print UI status flags : " << result << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Wait for glasses
    {
        auto glasses = waitForService<Glasses>(*client, waitForGlasses);
        if (!glasses)
        {
            std::cerr << "Failed to wait for glasses : " << glasses << std::endl;
            std::exit(EXIT_FAILURE);
        }

        // Add the glasses to the parameter change listener
        paramChangeHelper->registerGlasses(*glasses);

        // Do things with the glasses
        result = doThingsWithGlasses(*glasses);
        if (!result)
        {
            std::cerr << "Failed to do things with glasses : " << result << std::endl;
        }
    }

    std::cout << "Waiting a little..." << std::endl;
    std::this_thread::sleep_for(5000_ms);

    std::cout << "ALL DONE!" << std::endl;
}
/// [Main]
