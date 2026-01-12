#ifndef IMAGE_SOURCE_INTERFACE_HPP
#define IMAGE_SOURCE_INTERFACE_HPP

#include <opencv2/core.hpp>
#include <string>

class IImageSource
{
public:
    enum class GrabMode
    {
        Once,
        Timed,
        Continuous
    };

    IImageSource() = default;
    virtual ~IImageSource() = default;

    IImageSource(const IImageSource&) = delete;
    IImageSource& operator=(const IImageSource&) = delete;
    IImageSource(IImageSource&&) = default;
    IImageSource& operator=(IImageSource&&) = default;

    /** @brief Starts the stream from the source */
    virtual void start() = 0;
    /** @brief Stops the stream from the source */
    virtual void close() = 0;

    /** @brief Captures a single frame from the source */
    virtual cv::Mat grab() = 0;

    /** @brief Returns the frame capture mode of the source
     * 
     * GrabMode defines how the source provides frames:  
     *   - Once: the source provides a single frame on request and then stops.
     *   - Timed: the source provides frames without blocking; polling at a defined interval is required.
     *   - Continuous: the source operates with blocking; external timeouts are not required.
     * 
     */
    virtual GrabMode grab_mode() = 0;

    /** @brief Returns a string with information about the source */
    virtual std::string get_info() const = 0;
};

#endif
