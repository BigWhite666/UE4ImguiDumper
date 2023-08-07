// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html
#include "test_precomp.hpp"

namespace opencv_test { namespace {

#ifdef HAVE_TIFF

// these defines are used to resolve conflict between tiff.h and opencv2/core/types_c.h
#define uint64 uint64_hack_
#define int64 int64_hack_
#include "tiff.h"

#ifdef __ANDROID__
// Test disabled as it uses a lot of memory.
// It is killed with SIGKILL by out of memory killer.
TEST(Imgcodecs_Tiff, DISABLED_decode_tile16384x16384)
#else
TEST(Imgcodecs_Tiff, decode_tile16384x16384)
#endif
{
    // see issue #2161
    cv::Mat big(16384, 16384, CV_8UC1, cv::Scalar::all(0));
    string file3 = cv::tempfile(".tiff");
    string file4 = cv::tempfile(".tiff");

    std::vector<int> params;
    params.push_back(TIFFTAG_ROWSPERSTRIP);
    params.push_back(big.rows);
    EXPECT_NO_THROW(cv::imwrite(file4, big, params));
    EXPECT_NO_THROW(cv::imwrite(file3, big.colRange(0, big.cols - 1), params));
    big.release();

    try
    {
        cv::imread(file3, IMREAD_UNCHANGED);
        EXPECT_NO_THROW(cv::imread(file4, IMREAD_UNCHANGED));
    }
    catch(const std::bad_alloc&)
    {
        // not enough memory
    }

    EXPECT_EQ(0, remove(file3.c_str()));
    EXPECT_EQ(0, remove(file4.c_str()));
}

TEST(Imgcodecs_Tiff, write_read_16bit_big_little_endian)
{
    // see issue #2601 "16-bit Grayscale TIFF Load Failures Due to Buffer Underflow and Endianness"

    // Setup data for two minimal 16-bit grayscale TIFF files in both endian formats
    uchar tiff_sample_data[2][86] = { {
        // Little endian
        0x49, 0x49, 0x2a, 0x00, 0x0c, 0x00, 0x00, 0x00, 0xad, 0xde, 0xef, 0xbe, 0x06, 0x00, 0x00, 0x01,
        0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00,
        0x00, 0x00, 0x06, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x11, 0x01,
        0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x17, 0x01, 0x04, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x04, 0x00, 0x00, 0x00 }, {
        // Big endian
        0x4d, 0x4d, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x0c, 0xde, 0xad, 0xbe, 0xef, 0x00, 0x06, 0x01, 0x00,
        0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x01, 0x01, 0x00, 0x03, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x02, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x10,
        0x00, 0x00, 0x01, 0x06, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x11,
        0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x01, 0x17, 0x00, 0x04, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x04 }
        };

    // Test imread() for both a little endian TIFF and big endian TIFF
    for (int i = 0; i < 2; i++)
    {
        string filename = cv::tempfile(".tiff");

        // Write sample TIFF file
        FILE* fp = fopen(filename.c_str(), "wb");
        ASSERT_TRUE(fp != NULL);
        ASSERT_EQ((size_t)1, fwrite(tiff_sample_data[i], 86, 1, fp));
        fclose(fp);

        Mat img = imread(filename, IMREAD_UNCHANGED);

        EXPECT_EQ(1, img.rows);
        EXPECT_EQ(2, img.cols);
        EXPECT_EQ(CV_16U, img.type());
        EXPECT_EQ(sizeof(ushort), img.elemSize());
        EXPECT_EQ(1, img.channels());
        EXPECT_EQ(0xDEAD, img.at<ushort>(0,0));
        EXPECT_EQ(0xBEEF, img.at<ushort>(0,1));

        EXPECT_EQ(0, remove(filename.c_str()));
    }
}

TEST(Imgcodecs_Tiff, decode_tile_remainder)
{
    /* see issue #3472 - dealing with tiled images where the tile size is
     * not a multiple of image size.
     * The tiled images were created with 'convert' from ImageMagick,
     * using the command 'convert <input> -define tiff:tile-geometry=128x128 -depth [8|16] <output>
     * Note that the conversion to 16 bits expands the range from 0-255 to 0-255*255,
     * so the test converts back but rounding errors cause small differences.
     */
    const string root = cvtest::TS::ptr()->get_data_path();
    cv::Mat img = imread(root + "readwrite/non_tiled.tif",-1);
    ASSERT_FALSE(img.empty());
    ASSERT_TRUE(img.channels() == 3);
    cv::Mat tiled8 = imread(root + "readwrite/tiled_8.tif", -1);
    ASSERT_FALSE(tiled8.empty());
    ASSERT_PRED_FORMAT2(cvtest::MatComparator(0, 0), img, tiled8);
    cv::Mat tiled16 = imread(root + "readwrite/tiled_16.tif", -1);
    ASSERT_FALSE(tiled16.empty());
    ASSERT_TRUE(tiled16.elemSize() == 6);
    tiled16.convertTo(tiled8, CV_8UC3, 1./256.);
    ASSERT_PRED_FORMAT2(cvtest::MatComparator(2, 0), img, tiled8);
    // What about 32, 64 bit?
}

TEST(Imgcodecs_Tiff, decode_10_12_14)
{
    /* see issue #21700
    */
    const string root = cvtest::TS::ptr()->get_data_path();

    const double maxDiff = 256;//samples do not have the exact same values because of the tool that created them
    cv::Mat tmp;
    double diff = 0;

    cv::Mat img8UC1 = imread(root + "readwrite/pattern_8uc1.tif", cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(img8UC1.empty());
    ASSERT_EQ(img8UC1.type(), CV_8UC1);

    cv::Mat img8UC3 = imread(root + "readwrite/pattern_8uc3.tif", cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(img8UC3.empty());
    ASSERT_EQ(img8UC3.type(), CV_8UC3);

    cv::Mat img8UC4 = imread(root + "readwrite/pattern_8uc4.tif", cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(img8UC4.empty());
    ASSERT_EQ(img8UC4.type(), CV_8UC4);

    cv::Mat img16UC1 = imread(root + "readwrite/pattern_16uc1.tif", cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(img16UC1.empty());
    ASSERT_EQ(img16UC1.type(), CV_16UC1);
    ASSERT_EQ(img8UC1.size(), img16UC1.size());
    img8UC1.convertTo(tmp, img16UC1.type(), (1U<<(16-8)));
    diff = cv::norm(tmp.reshape(1), img16UC1.reshape(1), cv::NORM_INF);
    ASSERT_LE(diff, maxDiff);

    cv::Mat img16UC3 = imread(root + "readwrite/pattern_16uc3.tif", cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(img16UC3.empty());
    ASSERT_EQ(img16UC3.type(), CV_16UC3);
    ASSERT_EQ(img8UC3.size(), img16UC3.size());
    img8UC3.convertTo(tmp, img16UC3.type(), (1U<<(16-8)));
    diff = cv::norm(tmp.reshape(1), img16UC3.reshape(1), cv::NORM_INF);
    ASSERT_LE(diff, maxDiff);

    cv::Mat img16UC4 = imread(root + "readwrite/pattern_16uc4.tif", cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(img16UC4.empty());
    ASSERT_EQ(img16UC4.type(), CV_16UC4);
    ASSERT_EQ(img8UC4.size(), img16UC4.size());
    img8UC4.convertTo(tmp, img16UC4.type(), (1U<<(16-8)));
    diff = cv::norm(tmp.reshape(1), img16UC4.reshape(1), cv::NORM_INF);
    ASSERT_LE(diff, maxDiff);

    cv::Mat img10UC1 = imread(root + "readwrite/pattern_10uc1.tif", cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(img10UC1.empty());
    ASSERT_EQ(img10UC1.type(), CV_16UC1);
    ASSERT_EQ(img10UC1.size(), img16UC1.size());
    diff = cv::norm(img10UC1.reshape(1), img16UC1.reshape(1), cv::NORM_INF);
    ASSERT_LE(diff, maxDiff);

    cv::Mat img10UC3 = imread(root + "readwrite/pattern_10uc3.tif", cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(img10UC3.empty());
    ASSERT_EQ(img10UC3.type(), CV_16UC3);
    ASSERT_EQ(img10UC3.size(), img16UC3.size());
    diff = cv::norm(img10UC3.reshape(1), img16UC3.reshape(1), cv::NORM_INF);
    ASSERT_LE(diff, maxDiff);

    cv::Mat img10UC4 = imread(root + "readwrite/pattern_10uc4.tif", cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(img10UC4.empty());
    ASSERT_EQ(img10UC4.type(), CV_16UC4);
    ASSERT_EQ(img10UC4.size(), img16UC4.size());
    diff = cv::norm(img10UC4.reshape(1), img16UC4.reshape(1), cv::NORM_INF);
    ASSERT_LE(diff, maxDiff);

    cv::Mat img12UC1 = imread(root + "readwrite/pattern_12uc1.tif", cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(img12UC1.empty());
    ASSERT_EQ(img12UC1.type(), CV_16UC1);
    ASSERT_EQ(img12UC1.size(), img16UC1.size());
    diff = cv::norm(img12UC1.reshape(1), img16UC1.reshape(1), cv::NORM_INF);
    ASSERT_LE(diff, maxDiff);

    cv::Mat img12UC3 = imread(root + "readwrite/pattern_12uc3.tif", cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(img12UC3.empty());
    ASSERT_EQ(img12UC3.type(), CV_16UC3);
    ASSERT_EQ(img12UC3.size(), img16UC3.size());
    diff = cv::norm(img12UC3.reshape(1), img16UC3.reshape(1), cv::NORM_INF);
    ASSERT_LE(diff, maxDiff);

    cv::Mat img12UC4 = imread(root + "readwrite/pattern_12uc4.tif", cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(img12UC4.empty());
    ASSERT_EQ(img12UC4.type(), CV_16UC4);
    ASSERT_EQ(img12UC4.size(), img16UC4.size());
    diff = cv::norm(img12UC4.reshape(1), img16UC4.reshape(1), cv::NORM_INF);
    ASSERT_LE(diff, maxDiff);

    cv::Mat img14UC1 = imread(root + "readwrite/pattern_14uc1.tif", cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(img14UC1.empty());
    ASSERT_EQ(img14UC1.type(), CV_16UC1);
    ASSERT_EQ(img14UC1.size(), img16UC1.size());
    diff = cv::norm(img14UC1.reshape(1), img16UC1.reshape(1), cv::NORM_INF);
    ASSERT_LE(diff, maxDiff);

    cv::Mat img14UC3 = imread(root + "readwrite/pattern_14uc3.tif", cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(img14UC3.empty());
    ASSERT_EQ(img14UC3.type(), CV_16UC3);
    ASSERT_EQ(img14UC3.size(), img16UC3.size());
    diff = cv::norm(img14UC3.reshape(1), img16UC3.reshape(1), cv::NORM_INF);
    ASSERT_LE(diff, maxDiff);

    cv::Mat img14UC4 = imread(root + "readwrite/pattern_14uc4.tif", cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(img14UC4.empty());
    ASSERT_EQ(img14UC4.type(), CV_16UC4);
    ASSERT_EQ(img14UC4.size(), img16UC4.size());
    diff = cv::norm(img14UC4.reshape(1), img16UC4.reshape(1), cv::NORM_INF);
    ASSERT_LE(diff, maxDiff);
}

TEST(Imgcodecs_Tiff, decode_infinite_rowsperstrip)
{
    const uchar sample_data[142] = {
        0x49, 0x49, 0x2a, 0x00, 0x10, 0x00, 0x00, 0x00, 0x56, 0x54,
        0x56, 0x5a, 0x59, 0x55, 0x5a, 0x00, 0x0a, 0x00, 0x00, 0x01,
        0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00,
        0x00, 0x00, 0x02, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x03, 0x01, 0x03, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x01, 0x03, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x11, 0x01,
        0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
        0x15, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x16, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0x17, 0x01, 0x04, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x1c, 0x01, 0x03, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00
    };

    const string filename = cv::tempfile(".tiff");
    std::ofstream outfile(filename.c_str(), std::ofstream::binary);
    outfile.write(reinterpret_cast<const char *>(sample_data), sizeof sample_data);
    outfile.close();

    EXPECT_NO_THROW(cv::imread(filename, IMREAD_UNCHANGED));

    EXPECT_EQ(0, remove(filename.c_str()));
}

TEST(Imgcodecs_Tiff, readWrite_unsigned)
{
    const string root = cvtest::TS::ptr()->get_data_path();
    const string filenameInput = root + "readwrite/gray_8u.tif";
    const string filenameOutput = cv::tempfile(".tiff");
    const Mat img = cv::imread(filenameInput, IMREAD_UNCHANGED);
    ASSERT_FALSE(img.empty());
    ASSERT_EQ(CV_8UC1, img.type());

    Mat matS8;
    img.convertTo(matS8, CV_8SC1);

    ASSERT_TRUE(cv::imwrite(filenameOutput, matS8));
    const Mat img2 = cv::imread(filenameOutput, IMREAD_UNCHANGED);
    ASSERT_EQ(img2.type(), matS8.type());
    ASSERT_EQ(img2.size(), matS8.size());
    EXPECT_LE(cvtest::norm(matS8, img2, NORM_INF | NORM_RELATIVE), 1e-3);
    EXPECT_EQ(0, remove(filenameOutput.c_str()));
}

TEST(Imgcodecs_Tiff, readWrite_32FC1)
{
    const string root = cvtest::TS::ptr()->get_data_path();
    const string filenameInput = root + "readwrite/test32FC1.tiff";
    const string filenameOutput = cv::tempfile(".tiff");
    const Mat img = cv::imread(filenameInput, IMREAD_UNCHANGED);
    ASSERT_FALSE(img.empty());
    ASSERT_EQ(CV_32FC1,img.type());

    ASSERT_TRUE(cv::imwrite(filenameOutput, img));
    const Mat img2 = cv::imread(filenameOutput, IMREAD_UNCHANGED);
    ASSERT_EQ(img2.type(), img.type());
    ASSERT_EQ(img2.size(), img.size());
    EXPECT_LE(cvtest::norm(img, img2, NORM_INF | NORM_RELATIVE), 1e-3);
    EXPECT_EQ(0, remove(filenameOutput.c_str()));
}

TEST(Imgcodecs_Tiff, readWrite_64FC1)
{
    const string root = cvtest::TS::ptr()->get_data_path();
    const string filenameInput = root + "readwrite/test64FC1.tiff";
    const string filenameOutput = cv::tempfile(".tiff");
    const Mat img = cv::imread(filenameInput, IMREAD_UNCHANGED);
    ASSERT_FALSE(img.empty());
    ASSERT_EQ(CV_64FC1, img.type());

    ASSERT_TRUE(cv::imwrite(filenameOutput, img));
    const Mat img2 = cv::imread(filenameOutput, IMREAD_UNCHANGED);
    ASSERT_EQ(img2.type(), img.type());
    ASSERT_EQ(img2.size(), img.size());
    EXPECT_LE(cvtest::norm(img, img2, NORM_INF | NORM_RELATIVE), 1e-3);
    EXPECT_EQ(0, remove(filenameOutput.c_str()));
}

TEST(Imgcodecs_Tiff, readWrite_32FC3_SGILOG)
{
    const string root = cvtest::TS::ptr()->get_data_path();
    const string filenameInput = root + "readwrite/test32FC3_sgilog.tiff";
    const string filenameOutput = cv::tempfile(".tiff");
    const Mat img = cv::imread(filenameInput, IMREAD_UNCHANGED);
    ASSERT_FALSE(img.empty());
    ASSERT_EQ(CV_32FC3, img.type());

    ASSERT_TRUE(cv::imwrite(filenameOutput, img));
    const Mat img2 = cv::imread(filenameOutput, IMREAD_UNCHANGED);
    ASSERT_EQ(img2.type(), img.type());
    ASSERT_EQ(img2.size(), img.size());
    EXPECT_LE(cvtest::norm(img, img2, NORM_INF | NORM_RELATIVE), 0.01);
    EXPECT_EQ(0, remove(filenameOutput.c_str()));
}

TEST(Imgcodecs_Tiff, readWrite_32FC3_RAW)
{
    const string root = cvtest::TS::ptr()->get_data_path();
    const string filenameInput = root + "readwrite/test32FC3_raw.tiff";
    const string filenameOutput = cv::tempfile(".tiff");
    const Mat img = cv::imread(filenameInput, IMREAD_UNCHANGED);
    ASSERT_FALSE(img.empty());
    ASSERT_EQ(CV_32FC3, img.type());

    std::vector<int> params;
    params.push_back(IMWRITE_TIFF_COMPRESSION);
    params.push_back(1/*COMPRESSION_NONE*/);

    ASSERT_TRUE(cv::imwrite(filenameOutput, img, params));
    const Mat img2 = cv::imread(filenameOutput, IMREAD_UNCHANGED);
    ASSERT_EQ(img2.type(), img.type());
    ASSERT_EQ(img2.size(), img.size());
    EXPECT_LE(cvtest::norm(img, img2, NORM_INF | NORM_RELATIVE), 1e-3);
    EXPECT_EQ(0, remove(filenameOutput.c_str()));
}

TEST(Imgcodecs_Tiff, read_palette_color_image)
{
    const string root = cvtest::TS::ptr()->get_data_path();
    const string filenameInput = root + "readwrite/test_palette_color_image.tif";

    const Mat img = cv::imread(filenameInput, IMREAD_UNCHANGED);
    ASSERT_FALSE(img.empty());
    ASSERT_EQ(CV_8UC3, img.type());
}

TEST(Imgcodecs_Tiff, read_4_bit_palette_color_image)
{
    const string root = cvtest::TS::ptr()->get_data_path();
    const string filenameInput = root + "readwrite/4-bit_palette_color.tif";

    const Mat img = cv::imread(filenameInput, IMREAD_UNCHANGED);
    ASSERT_FALSE(img.empty());
    ASSERT_EQ(CV_8UC3, img.type());
}

TEST(Imgcodecs_Tiff, readWrite_predictor)
{
    /* see issue #21871
     */
    const uchar sample_data[160] = {
        0xff, 0xff, 0xff, 0xff, 0x88, 0x88, 0xff, 0xff, 0x88, 0x88, 0xff, 0xff, 0xff, 0xff, 0xff, 0x88,
        0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x44, 0xff, 0xff, 0x88, 0xff, 0x33, 0x00, 0x66, 0xff, 0xff, 0x88, 0x00, 0x44,
        0x88, 0x00, 0x44, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x44, 0xff, 0xff, 0x11, 0x00, 0xff,
        0x11, 0x00, 0x88, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff,
        0x11, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x33, 0x00, 0x88, 0xff, 0x00, 0x66, 0xff,
        0x11, 0x00, 0x66, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x44, 0x33, 0x00, 0xff, 0xff,
        0x88, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
        0xff, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x33, 0x00, 0x00, 0x66, 0xff, 0xff,
        0xff, 0xff, 0x88, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff
    };

    cv::Mat mat(10, 16, CV_8UC1, (void*)sample_data);
    int methods[] = {
        COMPRESSION_NONE,     COMPRESSION_LZW,
        COMPRESSION_PACKBITS, COMPRESSION_DEFLATE,  COMPRESSION_ADOBE_DEFLATE
    };
    for (size_t i = 0; i < sizeof(methods) / sizeof(int); i++)
    {
        string out = cv::tempfile(".tif");

        std::vector<int> params;
        params.push_back(TIFFTAG_COMPRESSION);
        params.push_back(methods[i]);
        params.push_back(TIFFTAG_PREDICTOR);
        params.push_back(PREDICTOR_HORIZONTAL);

        EXPECT_NO_THROW(cv::imwrite(out, mat, params));

        const Mat img = cv::imread(out, IMREAD_UNCHANGED);
        ASSERT_FALSE(img.empty());

        ASSERT_EQ(0, cv::norm(mat, img, cv::NORM_INF));

        EXPECT_EQ(0, remove(out.c_str()));
    }
}


//==================================================================================================

typedef testing::TestWithParam<int> Imgcodecs_Tiff_Modes;

TEST_P(Imgcodecs_Tiff_Modes, decode_multipage)
{
    const int mode = GetParam();
    const string root = cvtest::TS::ptr()->get_data_path();
    const string filename = root + "readwrite/multipage.tif";
    const string page_files[] = {
        "readwrite/multipage_p1.tif",
        "readwrite/multipage_p2.tif",
        "readwrite/multipage_p3.tif",
        "readwrite/multipage_p4.tif",
        "readwrite/multipage_p5.tif",
        "readwrite/multipage_p6.tif"
    };
    const size_t page_count = sizeof(page_files)/sizeof(page_files[0]);
    vector<Mat> pages;
    bool res = imreadmulti(filename, pages, mode);
    ASSERT_TRUE(res == true);
    ASSERT_EQ(page_count, pages.size());
    for (size_t i = 0; i < page_count; i++)
    {
        const Mat page = imread(root + page_files[i], mode);
        EXPECT_PRED_FORMAT2(cvtest::MatComparator(0, 0), page, pages[i]);
    }
}

const int all_modes[] =
{
    IMREAD_UNCHANGED,
    IMREAD_GRAYSCALE,
    IMREAD_COLOR,
    IMREAD_ANYDEPTH,
    IMREAD_ANYCOLOR
};

INSTANTIATE_TEST_CASE_P(AllModes, Imgcodecs_Tiff_Modes, testing::ValuesIn(all_modes));

//==================================================================================================

TEST(Imgcodecs_Tiff_Modes, write_multipage)
{
    const string root = cvtest::TS::ptr()->get_data_path();
    const string filename = root + "readwrite/multipage.tif";
    const string page_files[] = {
        "readwrite/multipage_p1.tif",
        "readwrite/multipage_p2.tif",
        "readwrite/multipage_p3.tif",
        "readwrite/multipage_p4.tif",
        "readwrite/multipage_p5.tif",
        "readwrite/multipage_p6.tif"
    };
    const size_t page_count = sizeof(page_files) / sizeof(page_files[0]);
    vector<Mat> pages;
    for (size_t i = 0; i < page_count; i++)
    {
        const Mat page = imread(root + page_files[i]);
        pages.push_back(page);
    }

    string tmp_filename = cv::tempfile(".tiff");
    bool res = imwrite(tmp_filename, pages);
    ASSERT_TRUE(res);

    vector<Mat> read_pages;
    imreadmulti(tmp_filename, read_pages);
    for (size_t i = 0; i < page_count; i++)
    {
        EXPECT_PRED_FORMAT2(cvtest::MatComparator(0, 0), read_pages[i], pages[i]);
    }
}

//==================================================================================================

TEST(Imgcodecs_Tiff, imdecode_no_exception_temporary_file_removed)
{
    const string root = cvtest::TS::ptr()->get_data_path();
    const string filename = root + "../cv/shared/lena.png";
    cv::Mat img = cv::imread(filename);
    ASSERT_FALSE(img.empty());
    std::vector<uchar> buf;
    EXPECT_NO_THROW(cv::imencode(".tiff", img, buf));
    EXPECT_NO_THROW(cv::imdecode(buf, IMREAD_UNCHANGED));
}


TEST(Imgcodecs_Tiff, decode_black_and_write_image_pr12989_grayscale)
{
    const string filename = cvtest::findDataFile("readwrite/bitsperpixel1.tiff");
    cv::Mat img;
    ASSERT_NO_THROW(img = cv::imread(filename, IMREAD_GRAYSCALE));
    ASSERT_FALSE(img.empty());
    EXPECT_EQ(64, img.cols);
    EXPECT_EQ(64, img.rows);
    EXPECT_EQ(CV_8UC1, img.type()) << cv::typeToString(img.type());
    // Check for 0/255 values only: 267 + 3829 = 64*64
    EXPECT_EQ(267, countNonZero(img == 0));
    EXPECT_EQ(3829, countNonZero(img == 255));
}

TEST(Imgcodecs_Tiff, decode_black_and_write_image_pr12989_default)
{
    const string filename = cvtest::findDataFile("readwrite/bitsperpixel1.tiff");
    cv::Mat img;
    ASSERT_NO_THROW(img = cv::imread(filename));  // by default image type is CV_8UC3
    ASSERT_FALSE(img.empty());
    EXPECT_EQ(64, img.cols);
    EXPECT_EQ(64, img.rows);
    EXPECT_EQ(CV_8UC3, img.type()) << cv::typeToString(img.type());
}

TEST(Imgcodecs_Tiff, decode_black_and_write_image_pr17275_grayscale)
{
    const string filename = cvtest::findDataFile("readwrite/bitsperpixel1_min.tiff");
    cv::Mat img;
    ASSERT_NO_THROW(img = cv::imread(filename, IMREAD_GRAYSCALE));
    ASSERT_FALSE(img.empty());
    EXPECT_EQ(64, img.cols);
    EXPECT_EQ(64, img.rows);
    EXPECT_EQ(CV_8UC1, img.type()) << cv::typeToString(img.type());
    // Check for 0/255 values only: 267 + 3829 = 64*64
    EXPECT_EQ(267, countNonZero(img == 0));
    EXPECT_EQ(3829, countNonZero(img == 255));
}

TEST(Imgcodecs_Tiff, decode_black_and_write_image_pr17275_default)
{
    const string filename = cvtest::findDataFile("readwrite/bitsperpixel1_min.tiff");
    cv::Mat img;
    ASSERT_NO_THROW(img = cv::imread(filename));  // by default image type is CV_8UC3
    ASSERT_FALSE(img.empty());
    EXPECT_EQ(64, img.cols);
    EXPECT_EQ(64, img.rows);
    EXPECT_EQ(CV_8UC3, img.type()) << cv::typeToString(img.type());
}

TEST(Imgcodecs_Tiff, count_multipage)
{
    const string root = cvtest::TS::ptr()->get_data_path();
    {
        const string filename = root + "readwrite/multipage.tif";
        ASSERT_EQ((size_t)6, imcount(filename));
    }
    {
        const string filename = root + "readwrite/test32FC3_raw.tiff";
        ASSERT_EQ((size_t)1, imcount(filename));
    }
}

TEST(Imgcodecs_Tiff, read_multipage_indexed)
{
    const string root = cvtest::TS::ptr()->get_data_path();
    const string filename = root + "readwrite/multipage.tif";
    const string page_files[] = {
        "readwrite/multipage_p1.tif",
        "readwrite/multipage_p2.tif",
        "readwrite/multipage_p3.tif",
        "readwrite/multipage_p4.tif",
        "readwrite/multipage_p5.tif",
        "readwrite/multipage_p6.tif"
    };
    const int page_count = sizeof(page_files) / sizeof(page_files[0]);
    vector<Mat> single_pages;
    for (int i = 0; i < page_count; i++)
    {
        // imread and imreadmulti have different default values for the flag
        const Mat page = imread(root + page_files[i], IMREAD_ANYCOLOR);
        single_pages.push_back(page);
    }
    ASSERT_EQ((size_t)page_count, single_pages.size());

    {
        SCOPED_TRACE("Edge Cases");
        vector<Mat> multi_pages;
        bool res = imreadmulti(filename, multi_pages, 0, 0);
        // If we asked for 0 images and we successfully read 0 images should this be false ?
        ASSERT_TRUE(res == false);
        ASSERT_EQ((size_t)0, multi_pages.size());
        res = imreadmulti(filename, multi_pages, 0, 123123);
        ASSERT_TRUE(res == true);
        ASSERT_EQ((size_t)6, multi_pages.size());
    }

    {
        SCOPED_TRACE("Read all with indices");
        vector<Mat> multi_pages;
        bool res = imreadmulti(filename, multi_pages, 0, 6);
        ASSERT_TRUE(res == true);
        ASSERT_EQ((size_t)page_count, multi_pages.size());
        for (int i = 0; i < page_count; i++)
        {
            EXPECT_PRED_FORMAT2(cvtest::MatComparator(0, 0), multi_pages[i], single_pages[i]);
        }
    }

    {
        SCOPED_TRACE("Read one by one");
        vector<Mat> multi_pages;
        for (int i = 0; i < page_count; i++)
        {
            bool res = imreadmulti(filename, multi_pages, i, 1);
            ASSERT_TRUE(res == true);
            ASSERT_EQ((size_t)1, multi_pages.size());
            EXPECT_PRED_FORMAT2(cvtest::MatComparator(0, 0), multi_pages[0], single_pages[i]);
            multi_pages.clear();
        }
    }

    {
        SCOPED_TRACE("Read multiple at a time");
        vector<Mat> multi_pages;
        for (int i = 0; i < page_count/2; i++)
        {
            bool res = imreadmulti(filename, multi_pages, i*2, 2);
            ASSERT_TRUE(res == true);
            ASSERT_EQ((size_t)2, multi_pages.size());
            EXPECT_PRED_FORMAT2(cvtest::MatComparator(0, 0), multi_pages[0], single_pages[i * 2]) << i;
            EXPECT_PRED_FORMAT2(cvtest::MatComparator(0, 0), multi_pages[1], single_pages[i * 2 + 1]);
            multi_pages.clear();
        }
    }
}

TEST(Imgcodecs_Tiff, read_bigtiff_images)
{
    const string root = cvtest::TS::ptr()->get_data_path();
    const string filenamesInput[] = {
        "readwrite/BigTIFF.tif",
        "readwrite/BigTIFFMotorola.tif",
        "readwrite/BigTIFFLong.tif",
        "readwrite/BigTIFFLong8.tif",
        "readwrite/BigTIFFMotorolaLongStrips.tif",
        "readwrite/BigTIFFLong8Tiles.tif",
        "readwrite/BigTIFFSubIFD4.tif",
        "readwrite/BigTIFFSubIFD8.tif"
    };

    for (int i = 0; i < 8; i++)
    {
        const Mat bigtiff_img = imread(root + filenamesInput[i], IMREAD_UNCHANGED);
        ASSERT_FALSE(bigtiff_img.empty());
        EXPECT_EQ(64, bigtiff_img.cols);
        EXPECT_EQ(64, bigtiff_img.rows);
        ASSERT_EQ(CV_8UC3, bigtiff_img.type());
    }
}

#endif

}} // namespace
