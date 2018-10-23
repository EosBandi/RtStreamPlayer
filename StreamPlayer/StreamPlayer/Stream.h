#ifndef FFMPEG_FACADE_STREAM_H
#define FFMPEG_FACADE_STREAM_H

#include <cstdint>
#include <string>
#include <memory>
#include <chrono>
#include <boost/noncopyable.hpp>

#pragma warning( push )
#pragma warning( disable : 4100 )

#include <boost/thread.hpp>

#pragma warning( pop )

#include <boost/atomic.hpp>

#include "ConcurrentQueue.h"

namespace FFmpeg
{

#pragma warning( push )
#pragma warning( disable : 4244 )

	extern "C"
	{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
//#include <x264.h>
//#include <x264_config.h>

	}


//#pragma comment(lib, "libx264.lib")
#pragma comment(lib, "libgcc.a")
#pragma comment(lib, "bcrypt.lib")

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "postproc.lib")

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "Vfw32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Secur32.lib")

#pragma warning( pop )

	// The FFmpeg framework built using the following configuration:
	// x64: ./configure --toolchain=msvc --arch=amd64 --target-os=win64 --enable-gpl --enable-version3 --enable-static --disable-shared --disable-programs --disable-doc
    // x86: ./configure --toolchain=msvc --arch=i386 --enable-gpl --enable-version3 --enable-static --disable-shared --disable-programs --disable-doc 


	namespace Facade
	{
		enum RtspTransport : int32_t;
		enum RtspFlags : int32_t;
		enum FrameDelay : int32_t;

		class Frame;

		/// <summary>
		/// A Stream class converts a stream into a set of frames. 
		/// </summary>
		class Stream : private boost::noncopyable
		{
		public:
			/// <summary>
			/// Initializes a new instance of the Stream class.
			/// </summary>
			/// <param name="streamUrl">The url of a stream to decode.</param>
            /// <param name="connectionTimeoutInMilliseconds">The connection timeout in milliseconds.</param>
			/// <param name="transport">RTSP transport protocol.</param>
			/// <param name="flags">RTSP flags.</param>
            Stream(std::string const& streamUrl,  int32_t connectionTimeoutInMilliseconds,
				RtspTransport transport, RtspFlags flags, FrameDelay framedelay);

			/// <summary>
			/// Blocks the current thread until the stream gets opened or fails to open.
			/// </summary>
			void WaitForOpen();

			/// <summary>
			/// Gets the next frame in the stream.
			/// </summary>
			/// <returns>The next frame in the stream or nullptr if there are no more frames.</returns>
            std::unique_ptr<Frame> GetNextFrame();

			/// <summary>
			/// Gets an interframe delay, in milliseconds.
			/// </summary>
			int32_t GetInterframeDelayInMilliseconds(double frameTimestamp);

			/// <summary>
			/// Stops the stream.
			/// </summary>
            void Stop();

		private:			

            void Open();

            void Read();

            void OpenAndRead();

			bool IsOpen() const;

			double GetTimestamp(AVFrame *avframePtr, const AVRational& timeBase);

			std::unique_ptr<Frame> CreateFrame(AVFrame *avframePtr);

			std::unique_ptr<AVDictionary, std::function<void(AVDictionary*)>> GetOptions(RtspTransport transport, RtspFlags flags, FrameDelay framedelay);

            static int InterruptCallback(void *ctx);

			static std::string AvStrError(int errnum);

			std::string url_;
            std::chrono::milliseconds connectionTimeout_;
			RtspTransport transport_;
			RtspFlags flags_;
			FrameDelay framedelay_;
			bool openedOrFailed_;
			boost::atomic<bool> stopRequested_;
			int32_t videoStreamIndex_;

			double videoClock_; ///<pts of last decoded frame / predicted pts of next decoded frame
			double videoTimer_;			
			double lastFrameTimestamp_;
			double lastFrameDelay_;			

			std::unique_ptr<AVFormatContext, std::function<void(AVFormatContext*)>> formatContext_;
			std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> codecContext_;			
			std::unique_ptr<SwsContext, std::function<void(SwsContext*)>> imageConvertContext_;
			            
            std::string error_;			
            boost::thread workerThread_;
            boost::mutex mutex_;
            boost::condition_variable conditionVariable_;

            ConcurrentQueue<AVPacket *> packetQueue_;
            std::chrono::time_point<std::chrono::system_clock> connectionStart_;            
		};
	}
}

#endif // FFMPEG_FACADE_STREAM_H