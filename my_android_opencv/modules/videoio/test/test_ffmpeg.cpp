// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

#include "test_precomp.hpp"

using namespace std;

namespace opencv_test { namespace {

static inline long long getFileSize(const string &filename)
{
    ifstream f(filename, ios_base::in | ios_base::binary);
    f.seekg(0, ios_base::end);
    return f.tellg();
}

typedef tuple<string, string, Size> FourCC_Ext_Size;
typedef testing::TestWithParam< FourCC_Ext_Size > videoio_ffmpeg;

TEST_P(videoio_ffmpeg, write_big)
{
    if (!videoio_registry::hasBackend(CAP_FFMPEG))
        throw SkipTestException("FFmpeg backend was not found");

    const string fourcc = get<0>(GetParam());
    const string ext = get<1>(GetParam());
    const Size sz = get<2>(GetParam());
    const double time_sec = 1;
    const double fps = 25;

    ostringstream buf;
    buf << "write_big_" << fourcc << "." << ext;
    const string filename = tempfile(buf.str().c_str());

    VideoWriter writer(filename, CAP_FFMPEG, fourccFromString(fourcc), fps, sz);
    if (ext == "mp4" && fourcc == "H264" && !writer.isOpened())
    {
        throw cvtest::SkipTestException("H264/mp4 codec is not supported - SKIP");
    }
    ASSERT_TRUE(writer.isOpened());
    Mat img(sz, CV_8UC3, Scalar::all(0));
    const int coeff = cvRound(min(sz.width, sz.height)/(fps * time_sec));
    for (int i = 0 ; i < static_cast<int>(fps * time_sec); i++ )
    {
        rectangle(img,
                  Point2i(coeff * i, coeff * i),
                  Point2i(coeff * (i + 1), coeff * (i + 1)),
                  Scalar::all(255 * (1.0 - static_cast<double>(i) / (fps * time_sec * 2))),
                  -1);
        writer << img;
    }
    writer.release();
    EXPECT_GT(getFileSize(filename), 8192);
    remove(filename.c_str());
}

static const Size bigSize(4096, 4096);

const FourCC_Ext_Size entries[] =
{
    make_tuple("", "avi", bigSize),
    make_tuple("DX50", "avi", bigSize),
    make_tuple("FLV1", "avi", bigSize),
    make_tuple("H261", "avi", Size(352, 288)),
    make_tuple("H263", "avi", Size(704, 576)),
    make_tuple("I420", "avi", bigSize),
    make_tuple("MJPG", "avi", bigSize),
    make_tuple("mp4v", "avi", bigSize),
    make_tuple("MPEG", "avi", Size(720, 576)),
    make_tuple("XVID", "avi", bigSize),
    make_tuple("H264", "mp4", Size(4096, 2160))
};

INSTANTIATE_TEST_CASE_P(videoio, videoio_ffmpeg, testing::ValuesIn(entries));

//==========================================================================

TEST(videoio_ffmpeg, image)
{
    if (!videoio_registry::hasBackend(CAP_FFMPEG))
        throw SkipTestException("FFmpeg backend was not found");

    const string filename = findDataFile("readwrite/ordinary.bmp");
    Mat image = imread(filename, IMREAD_COLOR);
    ASSERT_FALSE(image.empty());
    VideoCapture cap(filename, CAP_FFMPEG);
    ASSERT_TRUE(cap.isOpened());
    Mat frame1, frame2;
    cap >> frame1 >> frame2;
    ASSERT_FALSE(frame1.empty());
    ASSERT_TRUE(frame2.empty());
    ASSERT_EQ(0, cvtest::norm(image, frame1, NORM_INF));
}

//==========================================================================

typedef tuple<VideoCaptureAPIs, string, string, string, string, string> videoio_container_params_t;
typedef testing::TestWithParam< videoio_container_params_t > videoio_container;

TEST_P(videoio_container, read)
{
    const VideoCaptureAPIs api = get<0>(GetParam());

    if (!videoio_registry::hasBackend(api))
        throw SkipTestException("Backend was not found");

    const string path = get<1>(GetParam());
    const string ext = get<2>(GetParam());
    const string ext_raw = get<3>(GetParam());
    const string codec = get<4>(GetParam());
    const string pixelFormat = get<5>(GetParam());
    const string fileName = path + "." + ext;
    const string fileNameOut = tempfile(cv::format("test_container_stream.%s", ext_raw.c_str()).c_str());

    // Write encoded video read using VideoContainer to tmp file
    size_t totalBytes = 0;
    {
        VideoCapture container(findDataFile(fileName), api);
        if (!container.isOpened())
            throw SkipTestException("Video stream is not supported");
        if (!container.set(CAP_PROP_FORMAT, -1))  // turn off video decoder (extract stream)
            throw SkipTestException("Fetching of RAW video streams is not supported");
        ASSERT_EQ(-1.f, container.get(CAP_PROP_FORMAT));  // check
        EXPECT_EQ(codec, fourccToString((int)container.get(CAP_PROP_FOURCC)));
        EXPECT_EQ(pixelFormat, fourccToString((int)container.get(CAP_PROP_CODEC_PIXEL_FORMAT)));

        std::ofstream file(fileNameOut.c_str(), ios::out | ios::trunc | std::ios::binary);
        Mat raw_data;
        while (true)
        {
            container >> raw_data;
            size_t size = raw_data.total();
            if (raw_data.empty())
                break;
            ASSERT_EQ(CV_8UC1, raw_data.type());
            ASSERT_LE(raw_data.dims, 2);
            ASSERT_EQ(raw_data.rows, 1);
            ASSERT_EQ((size_t)raw_data.cols, raw_data.total());
            ASSERT_TRUE(raw_data.isContinuous());
            totalBytes += size;
            file.write(reinterpret_cast<char*>(raw_data.data), size);
            ASSERT_FALSE(file.fail());
        }
        ASSERT_GE(totalBytes, (size_t)65536) << "Encoded stream is too small";
    }

    std::cout << "Checking extracted video stream: " << fileNameOut << " (size: " << totalBytes << " bytes)" << std::endl;

    // Check decoded frames read from original media are equal to frames decoded from tmp file
    {
        VideoCapture capReference(findDataFile(fileName), api);
        ASSERT_TRUE(capReference.isOpened());
        VideoCapture capActual(fileNameOut.c_str(), api);
        ASSERT_TRUE(capActual.isOpened());
        Mat reference, actual;
        int nframes = 0, n_err = 0;
        while (capReference.read(reference) && n_err < 3)
        {
            nframes++;
            ASSERT_TRUE(capActual.read(actual)) << nframes;
            EXPECT_EQ(0, cvtest::norm(actual, reference, NORM_INF)) << "frame=" << nframes << " err=" << ++n_err;
        }
        ASSERT_GT(nframes, 0);
    }

    ASSERT_EQ(0, remove(fileNameOut.c_str()));
}

const videoio_container_params_t videoio_container_params[] =
{
    videoio_container_params_t(CAP_FFMPEG, "video/big_buck_bunny", "h264", "h264", "h264", "I420"),
    videoio_container_params_t(CAP_FFMPEG, "video/big_buck_bunny", "h265", "h265", "hevc", "I420"),
    videoio_container_params_t(CAP_FFMPEG, "video/big_buck_bunny", "mjpg.avi", "mjpg", "MJPG", "I420"),
    //videoio_container_params_t(CAP_FFMPEG, "video/big_buck_bunny", "h264.mkv", "mkv.h264", "h264", "I420"),
    //videoio_container_params_t(CAP_FFMPEG, "video/big_buck_bunny", "h265.mkv", "mkv.h265", "hevc", "I420"),
    //videoio_container_params_t(CAP_FFMPEG, "video/big_buck_bunny", "h264.mp4", "mp4.avc1", "avc1", "I420"),
    //videoio_container_params_t(CAP_FFMPEG, "video/big_buck_bunny", "h265.mp4", "mp4.hev1", "hev1", "I420"),
};

INSTANTIATE_TEST_CASE_P(/**/, videoio_container, testing::ValuesIn(videoio_container_params));

typedef tuple<string, string, int> videoio_skip_params_t;
typedef testing::TestWithParam< videoio_skip_params_t > videoio_skip;

TEST_P(videoio_skip, DISABLED_read)  // optional test, may fail in some configurations
{
#if CV_VERSION_MAJOR >= 4
    if (!videoio_registry::hasBackend(CAP_FFMPEG))
        throw SkipTestException("Backend was not found");
#endif

    const string path = get<0>(GetParam());
    const string env = get<1>(GetParam());
    const int expectedFrameNumber = get<2>(GetParam());

    #ifdef _WIN32
        _putenv_s("OPENCV_FFMPEG_CAPTURE_OPTIONS", env.c_str());
    #else
        setenv("OPENCV_FFMPEG_CAPTURE_OPTIONS", env.c_str(), 1);
    #endif
    VideoCapture container(findDataFile(path), CAP_FFMPEG);
    #ifdef _WIN32
        _putenv_s("OPENCV_FFMPEG_CAPTURE_OPTIONS", "");
    #else
        setenv("OPENCV_FFMPEG_CAPTURE_OPTIONS", "", 1);
    #endif

    ASSERT_TRUE(container.isOpened());

    Mat reference;
    int nframes = 0, n_err = 0;
    while (container.isOpened())
    {
        if (container.read(reference))
            nframes++;
        else if (++n_err > 3)
            break;
    }
    EXPECT_EQ(expectedFrameNumber, nframes);
}

const videoio_skip_params_t videoio_skip_params[] =
{
    videoio_skip_params_t("video/big_buck_bunny.mp4", "", 125),
    videoio_skip_params_t("video/big_buck_bunny.mp4", "avdiscard;nonkey", 11)
};

INSTANTIATE_TEST_CASE_P(/**/, videoio_skip, testing::ValuesIn(videoio_skip_params));

//==========================================================================

static void generateFrame(Mat &frame, unsigned int i, const Point &center, const Scalar &color)
{
    frame = Scalar::all(i % 255);
    stringstream buf(ios::out);
    buf << "frame #" << i;
    putText(frame, buf.str(), Point(50, center.y), FONT_HERSHEY_SIMPLEX, 5.0, color, 5, LINE_AA);
    circle(frame, center, i + 2, color, 2, LINE_AA);
}

TEST(videoio_ffmpeg, parallel)
{
    if (!videoio_registry::hasBackend(CAP_FFMPEG))
        throw SkipTestException("FFmpeg backend was not found");

    const int NUM = 4;
    const int GRAN = 4;
    const Range R(0, NUM);
    const Size sz(1020, 900);
    const int frameNum = 300;
    const Scalar color(Scalar::all(0));
    const Point center(sz.height / 2, sz.width / 2);

    // Generate filenames
    vector<string> files;
    for (int i = 0; i < NUM; ++i)
    {
        ostringstream stream;
        stream << i << ".avi";
        files.push_back(tempfile(stream.str().c_str()));
    }
    // Write videos
    {
        vector< Ptr<VideoWriter> > writers(NUM);
        auto makeWriters = [&](const Range &r)
        {
            for (int i = r.start; i != r.end; ++i)
                writers[i] = makePtr<VideoWriter>(files[i],
                                                  CAP_FFMPEG,
                                                  VideoWriter::fourcc('X','V','I','D'),
                                                  25.0f,
                                                  sz);
        };
        parallel_for_(R, makeWriters, GRAN);
        for(int i = 0; i < NUM; ++i)
        {
            ASSERT_TRUE(writers[i]);
            ASSERT_TRUE(writers[i]->isOpened());
        }
        auto writeFrames = [&](const Range &r)
        {
            for (int j = r.start; j < r.end; ++j)
            {
                Mat frame(sz, CV_8UC3);
                for (int i = 0; i < frameNum; ++i)
                {
                    generateFrame(frame, i, center, color);
                    writers[j]->write(frame);
                }
            }
        };
        parallel_for_(R, writeFrames, GRAN);
    }
    // Read videos
    {
        vector< Ptr<VideoCapture> > readers(NUM);
        auto makeCaptures = [&](const Range &r)
        {
            for (int i = r.start; i != r.end; ++i)
                readers[i] = makePtr<VideoCapture>(files[i], CAP_FFMPEG);
        };
        parallel_for_(R, makeCaptures, GRAN);
        for(int i = 0; i < NUM; ++i)
        {
            ASSERT_TRUE(readers[i]);
            ASSERT_TRUE(readers[i]->isOpened());
        }
        auto readFrames = [&](const Range &r)
        {
            for (int j = r.start; j < r.end; ++j)
            {
                Mat reference(sz, CV_8UC3);
                for (int i = 0; i < frameNum; ++i)
                {
                    Mat actual;
                    EXPECT_TRUE(readers[j]->read(actual));
                    EXPECT_FALSE(actual.empty());
                    generateFrame(reference, i, center, color);
                    EXPECT_EQ(reference.size(), actual.size());
                    EXPECT_EQ(reference.depth(), actual.depth());
                    EXPECT_EQ(reference.channels(), actual.channels());
                    EXPECT_GE(cvtest::PSNR(actual, reference), 35.0) << "cap" << j << ", frame " << i;
                }
            }
        };
        parallel_for_(R, readFrames, GRAN);
    }
    // Remove files
    for(int i = 0; i < NUM; ++i)
    {
        remove(files[i].c_str());
    }
}

typedef std::pair<VideoCaptureProperties, double> cap_property_t;
typedef std::vector<cap_property_t> cap_properties_t;
typedef std::pair<std::string, cap_properties_t> ffmpeg_cap_properties_param_t;
typedef testing::TestWithParam<ffmpeg_cap_properties_param_t> ffmpeg_cap_properties;

#ifdef _WIN32
namespace {
::testing::AssertionResult IsOneOf(double value, double expected1, double expected2)
{
    // internal floating point class is used to perform accurate floating point types comparison
    typedef ::testing::internal::FloatingPoint<double> FloatingPoint;

    FloatingPoint val(value);
    if (val.AlmostEquals(FloatingPoint(expected1)) || val.AlmostEquals(FloatingPoint(expected2)))
    {
        return ::testing::AssertionSuccess();
    }
    else
    {
        return ::testing::AssertionFailure()
               << value << " is neither  equal to " << expected1 << " nor " << expected2;
    }
}
}
#endif

TEST_P(ffmpeg_cap_properties, can_read_property)
{
    if (!videoio_registry::hasBackend(CAP_FFMPEG))
        throw SkipTestException("FFmpeg backend was not found");

    ffmpeg_cap_properties_param_t parameters = GetParam();
    const std::string path = parameters.first;
    const cap_properties_t properties = parameters.second;

    VideoCapture cap(findDataFile(path), CAP_FFMPEG);
    ASSERT_TRUE(cap.isOpened()) << "Can not open " << findDataFile(path);

    for (std::size_t i = 0; i < properties.size(); ++i)
    {
        const cap_property_t& prop = properties[i];
        const double actualValue = cap.get(static_cast<int>(prop.first));
    #ifndef _WIN32
        EXPECT_DOUBLE_EQ(actualValue, prop.second)
            << "Property " << static_cast<int>(prop.first) << " has wrong value";
    #else
        EXPECT_TRUE(IsOneOf(actualValue, prop.second, 0.0))
            << "Property " << static_cast<int>(prop.first) << " has wrong value";
    #endif
    }
}

cap_properties_t loadBigBuckBunnyFFProbeResults() {
    cap_property_t properties[] = { cap_property_t(CAP_PROP_BITRATE, 5851.),
                                    cap_property_t(CAP_PROP_FPS, 24.),
                                    cap_property_t(CAP_PROP_FRAME_HEIGHT, 384.),
                                    cap_property_t(CAP_PROP_FRAME_WIDTH, 672.) };
    return cap_properties_t(properties, properties + sizeof(properties) / sizeof(cap_property_t));
}

const ffmpeg_cap_properties_param_t videoio_ffmpeg_properties[] = {
    ffmpeg_cap_properties_param_t("video/big_buck_bunny.avi", loadBigBuckBunnyFFProbeResults())
};

INSTANTIATE_TEST_CASE_P(videoio, ffmpeg_cap_properties, testing::ValuesIn(videoio_ffmpeg_properties));

typedef tuple<string, string> ffmpeg_get_fourcc_param_t;
typedef testing::TestWithParam<ffmpeg_get_fourcc_param_t> ffmpeg_get_fourcc;

TEST_P(ffmpeg_get_fourcc, check_short_codecs)
{
    const VideoCaptureAPIs api = CAP_FFMPEG;
    if (!videoio_registry::hasBackend(api))
        throw SkipTestException("Backend was not found");
    const string fileName = get<0>(GetParam());
    const string fourcc_string = get<1>(GetParam());
    VideoCapture cap(findDataFile(fileName), api);
    if (!cap.isOpened())
        throw SkipTestException("Video stream is not supported");
    const double fourcc = cap.get(CAP_PROP_FOURCC);
#ifdef _WIN32  // handle old FFmpeg backend
    if(!fourcc && fileName == "../cv/tracking/faceocc2/data/faceocc2.webm")
        throw SkipTestException("Feature not yet supported by Windows FFmpeg shared library!");
#endif
    ASSERT_EQ(fourccToString(fourcc), fourcc_string);
}

const ffmpeg_get_fourcc_param_t ffmpeg_get_fourcc_param[] =
{
    ffmpeg_get_fourcc_param_t("../cv/tracking/faceocc2/data/faceocc2.webm", "VP80"),
    ffmpeg_get_fourcc_param_t("video/sample_322x242_15frames.yuv420p.libvpx-vp9.mp4", "vp09"),
    ffmpeg_get_fourcc_param_t("video/sample_322x242_15frames.yuv420p.libaom-av1.mp4", "av01"),
    ffmpeg_get_fourcc_param_t("video/big_buck_bunny.h265", "hevc"),
    ffmpeg_get_fourcc_param_t("video/big_buck_bunny.h264", "h264")
};

INSTANTIATE_TEST_CASE_P(videoio, ffmpeg_get_fourcc, testing::ValuesIn(ffmpeg_get_fourcc_param));

// related issue: https://github.com/opencv/opencv/issues/15499
TEST(videoio, mp4_orientation_meta_auto)
{
    if (!videoio_registry::hasBackend(CAP_FFMPEG))
        throw SkipTestException("FFmpeg backend was not found");

    string video_file = string(cvtest::TS::ptr()->get_data_path()) + "video/big_buck_bunny_rotated.mp4";

    VideoCapture cap;
    EXPECT_NO_THROW(cap.open(video_file, CAP_FFMPEG));
    ASSERT_TRUE(cap.isOpened()) << "Can't open the video: " << video_file << " with backend " << CAP_FFMPEG << std::endl;

#ifndef _WIN32 // TODO: FFmpeg wrapper update
    // related issue: https://github.com/opencv/opencv/issues/22088
    EXPECT_EQ(90, cap.get(CAP_PROP_ORIENTATION_META));
#endif

    cap.set(CAP_PROP_ORIENTATION_AUTO, true);
    if (cap.get(CAP_PROP_ORIENTATION_AUTO) == 0)
        throw SkipTestException("FFmpeg frame rotation metadata is not supported");

    Size actual;
    EXPECT_NO_THROW(actual = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
                                    (int)cap.get(CAP_PROP_FRAME_HEIGHT)));
    EXPECT_EQ(384, actual.width);
    EXPECT_EQ(672, actual.height);

    Mat frame;

    cap >> frame;

    ASSERT_EQ(384, frame.cols);
    ASSERT_EQ(672, frame.rows);
}

// related issue: https://github.com/opencv/opencv/issues/15499
TEST(videoio, mp4_orientation_no_rotation)
{
    if (!videoio_registry::hasBackend(CAP_FFMPEG))
        throw SkipTestException("FFmpeg backend was not found");

    string video_file = string(cvtest::TS::ptr()->get_data_path()) + "video/big_buck_bunny_rotated.mp4";

    VideoCapture cap;
    EXPECT_NO_THROW(cap.open(video_file, CAP_FFMPEG));
    cap.set(CAP_PROP_ORIENTATION_AUTO, 0);
    ASSERT_TRUE(cap.isOpened()) << "Can't open the video: " << video_file << " with backend " << CAP_FFMPEG << std::endl;
    ASSERT_FALSE(cap.get(CAP_PROP_ORIENTATION_AUTO));

    Size actual;
    EXPECT_NO_THROW(actual = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
                                    (int)cap.get(CAP_PROP_FRAME_HEIGHT)));
    EXPECT_EQ(672, actual.width);
    EXPECT_EQ(384, actual.height);

    Mat frame;

    cap >> frame;

    ASSERT_EQ(672, frame.cols);
    ASSERT_EQ(384, frame.rows);
}


static void ffmpeg_check_read_raw(VideoCapture& cap)
{
    ASSERT_TRUE(cap.isOpened()) << "Can't open the video";

    Mat data;
    cap >> data;
    EXPECT_EQ(CV_8UC1, data.type()) << "CV_8UC1 != " << typeToString(data.type());
    EXPECT_TRUE(data.rows == 1 || data.cols == 1) << data.size;
    EXPECT_EQ((size_t)29729, data.total());

    cap >> data;
    EXPECT_EQ(CV_8UC1, data.type()) << "CV_8UC1 != " << typeToString(data.type());
    EXPECT_TRUE(data.rows == 1 || data.cols == 1) << data.size;
    EXPECT_EQ((size_t)37118, data.total());
}

TEST(videoio_ffmpeg, ffmpeg_check_extra_data)
{
    if (!videoio_registry::hasBackend(CAP_FFMPEG))
        throw SkipTestException("FFmpeg backend was not found");

    string video_file = findDataFile("video/big_buck_bunny.mp4");
    VideoCapture cap;
    EXPECT_NO_THROW(cap.open(video_file, CAP_FFMPEG));
    ASSERT_TRUE(cap.isOpened()) << "Can't open the video";
    const int codecExtradataIdx = (int)cap.get(CAP_PROP_CODEC_EXTRADATA_INDEX);
#ifdef _WIN32  // handle old FFmpeg backend
    if (codecExtradataIdx <= 0)
        throw SkipTestException("Codec extra data is not supported by backend or video stream");
#endif
    Mat data;
    ASSERT_TRUE(cap.retrieve(data, codecExtradataIdx));
    EXPECT_EQ(CV_8UC1, data.type()) << "CV_8UC1 != " << typeToString(data.type());
    EXPECT_TRUE(data.rows == 1 || data.cols == 1) << data.size;
    EXPECT_EQ((size_t)45, data.total());
}

TEST(videoio_ffmpeg, open_with_property)
{
    if (!videoio_registry::hasBackend(CAP_FFMPEG))
        throw SkipTestException("FFmpeg backend was not found");

    string video_file = findDataFile("video/big_buck_bunny.mp4");
    VideoCapture cap;
    EXPECT_NO_THROW(cap.open(video_file, CAP_FFMPEG, {
        CAP_PROP_FORMAT, -1  // demux only
    }));

    ffmpeg_check_read_raw(cap);
}

TEST(videoio_ffmpeg, create_with_property)
{
    if (!videoio_registry::hasBackend(CAP_FFMPEG))
        throw SkipTestException("FFmpeg backend was not found");

    string video_file = findDataFile("video/big_buck_bunny.mp4");
    VideoCapture cap(video_file, CAP_FFMPEG, {
        CAP_PROP_FORMAT, -1  // demux only
    });

    ffmpeg_check_read_raw(cap);
}

TEST(videoio_ffmpeg, create_with_property_badarg)
{
    if (!videoio_registry::hasBackend(CAP_FFMPEG))
        throw SkipTestException("FFmpeg backend was not found");

    string video_file = findDataFile("video/big_buck_bunny.mp4");
    VideoCapture cap(video_file, CAP_FFMPEG, {
        CAP_PROP_FORMAT, -2  // invalid
    });
    EXPECT_FALSE(cap.isOpened());
}

// related issue: https://github.com/opencv/opencv/issues/16821
TEST(videoio_ffmpeg, DISABLED_open_from_web)
{
    if (!videoio_registry::hasBackend(CAP_FFMPEG))
        throw SkipTestException("FFmpeg backend was not found");

    string video_file = "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4";
    VideoCapture cap(video_file, CAP_FFMPEG);
    int n_frames = -1;
    EXPECT_NO_THROW(n_frames = (int)cap.get(CAP_PROP_FRAME_COUNT));
    EXPECT_EQ((int)14315, n_frames);
}

}} // namespace
