// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

#include "test_precomp.hpp"

namespace opencv_test { namespace {

std::string qrcode_images_name[] = {
  "version_1_down.jpg", "version_1_left.jpg", "version_1_right.jpg", "version_1_up.jpg", "version_1_top.jpg",
  "version_2_down.jpg", "version_2_left.jpg", "version_2_right.jpg", "version_2_up.jpg", "version_2_top.jpg",
  "version_3_down.jpg", "version_3_left.jpg", "version_3_right.jpg", "version_3_up.jpg", "version_3_top.jpg",
  "version_4_down.jpg", "version_4_left.jpg", "version_4_right.jpg", "version_4_up.jpg", "version_4_top.jpg",
  "version_5_down.jpg", "version_5_left.jpg"/*"version_5_right.jpg"*/,
  "russian.jpg", "kanji.jpg", "link_github_ocv.jpg", "link_ocv.jpg", "link_wiki_cv.jpg"
// version_5_right.jpg DISABLED after tile fix, PR #22025
};

std::string qrcode_images_close[] = {
  "close_1.png", "close_2.png", "close_3.png", "close_4.png", "close_5.png"
};
std::string qrcode_images_monitor[] = {
  "monitor_1.png", "monitor_2.png", "monitor_3.png", "monitor_4.png", "monitor_5.png"
};
std::string qrcode_images_curved[] = {
  "curved_1.jpg", "curved_2.jpg", "curved_3.jpg", /*"curved_4.jpg",*/ "curved_5.jpg", /*"curved_6.jpg",*/ "curved_7.jpg", "curved_8.jpg"
};
// curved_4.jpg, curved_6.jpg DISABLED after tile fix, PR #22025
std::string qrcode_images_multiple[] = {
  "2_qrcodes.png", "3_close_qrcodes.png", "3_qrcodes.png", "4_qrcodes.png",
  "5_qrcodes.png", "6_qrcodes.png", "7_qrcodes.png", "8_close_qrcodes.png"
};
//#define UPDATE_QRCODE_TEST_DATA
#ifdef  UPDATE_QRCODE_TEST_DATA

TEST(Objdetect_QRCode, generate_test_data)
{
    const std::string root = "qrcode/";
    const std::string dataset_config = findDataFile(root + "dataset_config.json");
    FileStorage file_config(dataset_config, FileStorage::WRITE);

    file_config << "test_images" << "[";
    size_t images_count = sizeof(qrcode_images_name) / sizeof(qrcode_images_name[0]);
    for (size_t i = 0; i < images_count; i++)
    {
        file_config << "{:" << "image_name" << qrcode_images_name[i];
        std::string image_path = findDataFile(root + qrcode_images_name[i]);
        std::vector<Point> corners;
        Mat src = imread(image_path, IMREAD_GRAYSCALE), straight_barcode;
        std::string decoded_info;
        ASSERT_FALSE(src.empty()) << "Can't read image: " << image_path;
        EXPECT_TRUE(detectQRCode(src, corners));
#ifdef HAVE_QUIRC
        EXPECT_TRUE(decodeQRCode(src, corners, decoded_info, straight_barcode));
#endif
        file_config << "x" << "[:";
        for (size_t j = 0; j < corners.size(); j++) { file_config << corners[j].x; }
        file_config << "]";
        file_config << "y" << "[:";
        for (size_t j = 0; j < corners.size(); j++) { file_config << corners[j].y; }
        file_config << "]";
        file_config << "info" << decoded_info;
        file_config << "}";
    }
    file_config << "]";
    file_config.release();
}

TEST(Objdetect_QRCode_Close, generate_test_data)
{
    const std::string root = "qrcode/close/";
    const std::string dataset_config = findDataFile(root + "dataset_config.json");
    FileStorage file_config(dataset_config, FileStorage::WRITE);

    file_config << "close_images" << "[";
    size_t close_count = sizeof(qrcode_images_close) / sizeof(qrcode_images_close[0]);
    for (size_t i = 0; i < close_count; i++)
    {
        file_config << "{:" << "image_name" << qrcode_images_close[i];
        std::string image_path = findDataFile(root + qrcode_images_close[i]);
        std::vector<Point> corners;
        Mat src = imread(image_path, IMREAD_GRAYSCALE), barcode, straight_barcode;
        std::string decoded_info;
        ASSERT_FALSE(src.empty()) << "Can't read image: " << image_path;
        const double min_side = std::min(src.size().width, src.size().height);
        double coeff_expansion = 1024.0 / min_side;
        const int width  = cvRound(src.size().width  * coeff_expansion);
        const int height = cvRound(src.size().height  * coeff_expansion);
        Size new_size(width, height);
        resize(src, barcode, new_size, 0, 0, INTER_LINEAR);
        EXPECT_TRUE(detectQRCode(barcode, corners));
#ifdef HAVE_QUIRC
        EXPECT_TRUE(decodeQRCode(barcode, corners, decoded_info, straight_barcode));
#endif
        file_config << "x" << "[:";
        for (size_t j = 0; j < corners.size(); j++) { file_config << corners[j].x; }
        file_config << "]";
        file_config << "y" << "[:";
        for (size_t j = 0; j < corners.size(); j++) { file_config << corners[j].y; }
        file_config << "]";
        file_config << "info" << decoded_info;
        file_config << "}";
    }
    file_config << "]";
    file_config.release();
}
TEST(Objdetect_QRCode_Monitor, generate_test_data)
{
    const std::string root = "qrcode/monitor/";
    const std::string dataset_config = findDataFile(root + "dataset_config.json");
    FileStorage file_config(dataset_config, FileStorage::WRITE);

    file_config << "monitor_images" << "[";
    size_t monitor_count = sizeof(qrcode_images_monitor) / sizeof(qrcode_images_monitor[0]);
    for (size_t i = 0; i < monitor_count; i++)
    {
        file_config << "{:" << "image_name" << qrcode_images_monitor[i];
        std::string image_path = findDataFile(root + qrcode_images_monitor[i]);
        std::vector<Point> corners;
        Mat src = imread(image_path, IMREAD_GRAYSCALE), barcode, straight_barcode;
        std::string decoded_info;
        ASSERT_FALSE(src.empty()) << "Can't read image: " << image_path;
        const double min_side = std::min(src.size().width, src.size().height);
        double coeff_expansion = 1024.0 / min_side;
        const int width  = cvRound(src.size().width  * coeff_expansion);
        const int height = cvRound(src.size().height  * coeff_expansion);
        Size new_size(width, height);
        resize(src, barcode, new_size, 0, 0, INTER_LINEAR);
        EXPECT_TRUE(detectQRCode(barcode, corners));
#ifdef HAVE_QUIRC
        EXPECT_TRUE(decodeQRCode(barcode, corners, decoded_info, straight_barcode));
#endif
        file_config << "x" << "[:";
        for (size_t j = 0; j < corners.size(); j++) { file_config << corners[j].x; }
        file_config << "]";
        file_config << "y" << "[:";
        for (size_t j = 0; j < corners.size(); j++) { file_config << corners[j].y; }
        file_config << "]";
        file_config << "info" << decoded_info;
        file_config << "}";
    }
    file_config << "]";
    file_config.release();
}
TEST(Objdetect_QRCode_Curved, generate_test_data)
{
    const std::string root = "qrcode/curved/";
    const std::string dataset_config = findDataFile(root + "dataset_config.json");
    FileStorage file_config(dataset_config, FileStorage::WRITE);

    file_config << "test_images" << "[";
    size_t images_count = sizeof(qrcode_images_curved) / sizeof(qrcode_images_curved[0]);
    for (size_t i = 0; i < images_count; i++)
    {
        file_config << "{:" << "image_name" << qrcode_images_curved[i];
        std::string image_path = findDataFile(root + qrcode_images_curved[i]);
        std::vector<Point> corners;
        Mat src = imread(image_path, IMREAD_GRAYSCALE), straight_barcode;
        std::string decoded_info;
        ASSERT_FALSE(src.empty()) << "Can't read image: " << image_path;
        EXPECT_TRUE(detectQRCode(src, corners));
#ifdef HAVE_QUIRC
        EXPECT_TRUE(decodeCurvedQRCode(src, corners, decoded_info, straight_barcode));
#endif
        file_config << "x" << "[:";
        for (size_t j = 0; j < corners.size(); j++) { file_config << corners[j].x; }
        file_config << "]";
        file_config << "y" << "[:";
        for (size_t j = 0; j < corners.size(); j++) { file_config << corners[j].y; }
        file_config << "]";
        file_config << "info" << decoded_info;
        file_config << "}";
    }
    file_config << "]";
    file_config.release();
}
TEST(Objdetect_QRCode_Multi, generate_test_data)
{
    const std::string root = "qrcode/multiple/";
    const std::string dataset_config = findDataFile(root + "dataset_config.json");
    FileStorage file_config(dataset_config, FileStorage::WRITE);

    file_config << "multiple_images" << "[:";
    size_t multiple_count = sizeof(qrcode_images_multiple) / sizeof(qrcode_images_multiple[0]);
    for (size_t i = 0; i < multiple_count; i++)
    {
        file_config << "{:" << "image_name" << qrcode_images_multiple[i];
        std::string image_path = findDataFile(root + qrcode_images_multiple[i]);
        Mat src = imread(image_path);

        ASSERT_FALSE(src.empty()) << "Can't read image: " << image_path;
        std::vector<Point> corners;
        QRCodeDetector qrcode;
        EXPECT_TRUE(qrcode.detectMulti(src, corners));
#ifdef HAVE_QUIRC
        std::vector<cv::String> decoded_info;
        std::vector<Mat> straight_barcode;
        EXPECT_TRUE(qrcode.decodeMulti(src, corners, decoded_info, straight_barcode));
#endif
        file_config << "x" << "[:";
        for(size_t j = 0; j < corners.size(); j += 4)
        {
            file_config << "[:";
            for (size_t k = 0; k < 4; k++)
            {
                file_config << corners[j + k].x;
            }
            file_config << "]";
        }
        file_config << "]";
        file_config << "y" << "[:";
        for(size_t j = 0; j < corners.size(); j += 4)
        {
            file_config << "[:";
            for (size_t k = 0; k < 4; k++)
            {
                file_config << corners[j + k].y;
            }
            file_config << "]";
        }
        file_config << "]";
        file_config << "info";
        file_config << "[:";

        for(size_t j = 0; j < decoded_info.size(); j++)
        {
            file_config << decoded_info[j];
        }
        file_config << "]";
        file_config << "}";
    }

    file_config << "]";
    file_config.release();
}

#else

typedef testing::TestWithParam< std::string > Objdetect_QRCode;
TEST_P(Objdetect_QRCode, regression)
{
    const std::string name_current_image = GetParam();
    const std::string root = "qrcode/";
    const int pixels_error = 3;

    std::string image_path = findDataFile(root + name_current_image);
    Mat src = imread(image_path, IMREAD_GRAYSCALE), straight_barcode;
    ASSERT_FALSE(src.empty()) << "Can't read image: " << image_path;

    std::vector<Point> corners;
    std::string decoded_info;
    QRCodeDetector qrcode;
#ifdef HAVE_QUIRC
    decoded_info = qrcode.detectAndDecode(src, corners, straight_barcode);
    ASSERT_FALSE(corners.empty());
    ASSERT_FALSE(decoded_info.empty());
    int expected_barcode_type = CV_8UC1;
    EXPECT_EQ(expected_barcode_type, straight_barcode.type());
#else
    ASSERT_TRUE(qrcode.detect(src, corners));
#endif

    const std::string dataset_config = findDataFile(root + "dataset_config.json");
    FileStorage file_config(dataset_config, FileStorage::READ);
    ASSERT_TRUE(file_config.isOpened()) << "Can't read validation data: " << dataset_config;
    {
        FileNode images_list = file_config["test_images"];
        size_t images_count = static_cast<size_t>(images_list.size());
        ASSERT_GT(images_count, 0u) << "Can't find validation data entries in 'test_images': " << dataset_config;

        for (size_t index = 0; index < images_count; index++)
        {
            FileNode config = images_list[(int)index];
            std::string name_test_image = config["image_name"];
            if (name_test_image == name_current_image)
            {
                for (int i = 0; i < 4; i++)
                {
                    int x = config["x"][i];
                    int y = config["y"][i];
                    EXPECT_NEAR(x, corners[i].x, pixels_error);
                    EXPECT_NEAR(y, corners[i].y, pixels_error);
                }

#ifdef HAVE_QUIRC
                std::string original_info = config["info"];
                EXPECT_EQ(decoded_info, original_info);
#endif

                return; // done
            }
        }
        std::cerr
            << "Not found results for '" << name_current_image
            << "' image in config file:" << dataset_config << std::endl
            << "Re-run tests with enabled UPDATE_QRCODE_TEST_DATA macro to update test data."
            << std::endl;
    }
}

typedef testing::TestWithParam< std::string > Objdetect_QRCode_Close;
TEST_P(Objdetect_QRCode_Close, regression)
{
    const std::string name_current_image = GetParam();
    const std::string root = "qrcode/close/";
    const int pixels_error = 3;

    std::string image_path = findDataFile(root + name_current_image);
    Mat src = imread(image_path, IMREAD_GRAYSCALE), barcode, straight_barcode;
    ASSERT_FALSE(src.empty()) << "Can't read image: " << image_path;
    const double min_side = std::min(src.size().width, src.size().height);
    double coeff_expansion = 1024.0 / min_side;
    const int width  = cvRound(src.size().width  * coeff_expansion);
    const int height = cvRound(src.size().height  * coeff_expansion);
    Size new_size(width, height);
    resize(src, barcode, new_size, 0, 0, INTER_LINEAR);
    std::vector<Point> corners;
    std::string decoded_info;
    QRCodeDetector qrcode;
#ifdef HAVE_QUIRC
    decoded_info = qrcode.detectAndDecode(barcode, corners, straight_barcode);
    ASSERT_FALSE(corners.empty());
    ASSERT_FALSE(decoded_info.empty());
    int expected_barcode_type = CV_8UC1;
    EXPECT_EQ(expected_barcode_type, straight_barcode.type());
#else
    ASSERT_TRUE(qrcode.detect(barcode, corners));
#endif

    const std::string dataset_config = findDataFile(root + "dataset_config.json");
    FileStorage file_config(dataset_config, FileStorage::READ);
    ASSERT_TRUE(file_config.isOpened()) << "Can't read validation data: " << dataset_config;
    {
        FileNode images_list = file_config["close_images"];
        size_t images_count = static_cast<size_t>(images_list.size());
        ASSERT_GT(images_count, 0u) << "Can't find validation data entries in 'test_images': " << dataset_config;

        for (size_t index = 0; index < images_count; index++)
        {
            FileNode config = images_list[(int)index];
            std::string name_test_image = config["image_name"];
            if (name_test_image == name_current_image)
            {
                for (int i = 0; i < 4; i++)
                {
                    int x = config["x"][i];
                    int y = config["y"][i];
                    EXPECT_NEAR(x, corners[i].x, pixels_error);
                    EXPECT_NEAR(y, corners[i].y, pixels_error);
                }

#ifdef HAVE_QUIRC
                std::string original_info = config["info"];
                EXPECT_EQ(decoded_info, original_info);
#endif

                return; // done
            }
        }
        std::cerr
            << "Not found results for '" << name_current_image
            << "' image in config file:" << dataset_config << std::endl
            << "Re-run tests with enabled UPDATE_QRCODE_TEST_DATA macro to update test data."
            << std::endl;
    }
}

typedef testing::TestWithParam< std::string > Objdetect_QRCode_Monitor;
TEST_P(Objdetect_QRCode_Monitor, regression)
{
    const std::string name_current_image = GetParam();
    const std::string root = "qrcode/monitor/";
    const int pixels_error = 3;

    std::string image_path = findDataFile(root + name_current_image);
    Mat src = imread(image_path, IMREAD_GRAYSCALE), barcode, straight_barcode;
    ASSERT_FALSE(src.empty()) << "Can't read image: " << image_path;
    const double min_side = std::min(src.size().width, src.size().height);
    double coeff_expansion = 1024.0 / min_side;
    const int width  = cvRound(src.size().width  * coeff_expansion);
    const int height = cvRound(src.size().height  * coeff_expansion);
    Size new_size(width, height);
    resize(src, barcode, new_size, 0, 0, INTER_LINEAR);
    std::vector<Point> corners;
    std::string decoded_info;
    QRCodeDetector qrcode;
#ifdef HAVE_QUIRC
    decoded_info = qrcode.detectAndDecode(barcode, corners, straight_barcode);
    ASSERT_FALSE(corners.empty());
    ASSERT_FALSE(decoded_info.empty());
    int expected_barcode_type = CV_8UC1;
    EXPECT_EQ(expected_barcode_type, straight_barcode.type());
#else
    ASSERT_TRUE(qrcode.detect(barcode, corners));
#endif

    const std::string dataset_config = findDataFile(root + "dataset_config.json");
    FileStorage file_config(dataset_config, FileStorage::READ);
    ASSERT_TRUE(file_config.isOpened()) << "Can't read validation data: " << dataset_config;
    {
        FileNode images_list = file_config["monitor_images"];
        size_t images_count = static_cast<size_t>(images_list.size());
        ASSERT_GT(images_count, 0u) << "Can't find validation data entries in 'test_images': " << dataset_config;

        for (size_t index = 0; index < images_count; index++)
        {
            FileNode config = images_list[(int)index];
            std::string name_test_image = config["image_name"];
            if (name_test_image == name_current_image)
            {
                for (int i = 0; i < 4; i++)
                {
                    int x = config["x"][i];
                    int y = config["y"][i];
                    EXPECT_NEAR(x, corners[i].x, pixels_error);
                    EXPECT_NEAR(y, corners[i].y, pixels_error);
                }

#ifdef HAVE_QUIRC
                std::string original_info = config["info"];
                EXPECT_EQ(decoded_info, original_info);
#endif

                return; // done
            }
        }
        std::cerr
            << "Not found results for '" << name_current_image
            << "' image in config file:" << dataset_config << std::endl
            << "Re-run tests with enabled UPDATE_QRCODE_TEST_DATA macro to update test data."
            << std::endl;
    }
}

typedef testing::TestWithParam< std::string > Objdetect_QRCode_Curved;
TEST_P(Objdetect_QRCode_Curved, regression)
{
    const std::string name_current_image = GetParam();
    const std::string root = "qrcode/curved/";
    const int pixels_error = 3;

    std::string image_path = findDataFile(root + name_current_image);
    Mat src = imread(image_path, IMREAD_GRAYSCALE), straight_barcode;
    ASSERT_FALSE(src.empty()) << "Can't read image: " << image_path;

    std::vector<Point> corners;
    std::string decoded_info;
    QRCodeDetector qrcode;
#ifdef HAVE_QUIRC
    decoded_info = qrcode.detectAndDecodeCurved(src, corners, straight_barcode);
    ASSERT_FALSE(corners.empty());
    ASSERT_FALSE(decoded_info.empty());
    int expected_barcode_type = CV_8UC1;
    EXPECT_EQ(expected_barcode_type, straight_barcode.type());
#else
    ASSERT_TRUE(qrcode.detect(src, corners));
#endif

    const std::string dataset_config = findDataFile(root + "dataset_config.json");
    FileStorage file_config(dataset_config, FileStorage::READ);
    ASSERT_TRUE(file_config.isOpened()) << "Can't read validation data: " << dataset_config;
    {
        FileNode images_list = file_config["test_images"];
        size_t images_count = static_cast<size_t>(images_list.size());
        ASSERT_GT(images_count, 0u) << "Can't find validation data entries in 'test_images': " << dataset_config;

        for (size_t index = 0; index < images_count; index++)
        {
            FileNode config = images_list[(int)index];
            std::string name_test_image = config["image_name"];
            if (name_test_image == name_current_image)
            {
                for (int i = 0; i < 4; i++)
                {
                    int x = config["x"][i];
                    int y = config["y"][i];
                    EXPECT_NEAR(x, corners[i].x, pixels_error);
                    EXPECT_NEAR(y, corners[i].y, pixels_error);
                }

#ifdef HAVE_QUIRC
                std::string original_info = config["info"];
                EXPECT_EQ(decoded_info, original_info);
#endif

                return; // done
            }
        }
        std::cerr
            << "Not found results for '" << name_current_image
            << "' image in config file:" << dataset_config << std::endl
            << "Re-run tests with enabled UPDATE_QRCODE_TEST_DATA macro to update test data."
            << std::endl;
    }
}

typedef testing::TestWithParam < std::string > Objdetect_QRCode_Multi;
TEST_P(Objdetect_QRCode_Multi, regression)
{
    const std::string name_current_image = GetParam();
    const std::string root = "qrcode/multiple/";
    const int pixels_error = 3;

    std::string image_path = findDataFile(root + name_current_image);
    Mat src = imread(image_path);
    ASSERT_FALSE(src.empty()) << "Can't read image: " << image_path;
    QRCodeDetector qrcode;
    std::vector<Point> corners;
#ifdef HAVE_QUIRC
    std::vector<cv::String> decoded_info;
    std::vector<Mat> straight_barcode;
    EXPECT_TRUE(qrcode.detectAndDecodeMulti(src, decoded_info, corners, straight_barcode));
    ASSERT_FALSE(corners.empty());
    ASSERT_FALSE(decoded_info.empty());
    int expected_barcode_type = CV_8UC1;
    for(size_t i = 0; i < straight_barcode.size(); i++)
        EXPECT_EQ(expected_barcode_type, straight_barcode[i].type());
#else
    ASSERT_TRUE(qrcode.detectMulti(src, corners));
#endif

    const std::string dataset_config = findDataFile(root + "dataset_config.json");
    FileStorage file_config(dataset_config, FileStorage::READ);
    ASSERT_TRUE(file_config.isOpened()) << "Can't read validation data: " << dataset_config;
    {
        FileNode images_list = file_config["multiple_images"];
        size_t images_count = static_cast<size_t>(images_list.size());
        ASSERT_GT(images_count, 0u) << "Can't find validation data entries in 'test_images': " << dataset_config;
        for (size_t index = 0; index < images_count; index++)
        {
            FileNode config = images_list[(int)index];
            std::string name_test_image = config["image_name"];
            if (name_test_image == name_current_image)
            {
                for(int j = 0; j < int(corners.size()); j += 4)
                {
                    bool ok = false;
                    for (int k = 0; k < int(corners.size() / 4); k++)
                    {
                        int count_eq_points = 0;
                        for (int i = 0; i < 4; i++)
                        {
                            int x = config["x"][k][i];
                            int y = config["y"][k][i];
                            if(((abs(corners[j + i].x - x)) <= pixels_error) && ((abs(corners[j + i].y - y)) <= pixels_error))
                              count_eq_points++;
                        }
                        if (count_eq_points == 4)
                        {
                            ok = true;
                            break;
                        }
                    }
                    EXPECT_TRUE(ok);
                }

#ifdef HAVE_QUIRC
                  size_t count_eq_info = 0;
                  for(int i = 0; i < int(decoded_info.size()); i++)
                  {
                      for(int j = 0; j < int(decoded_info.size()); j++)
                      {
                          std::string original_info = config["info"][j];
                          if(original_info == decoded_info[i])
                          {
                             count_eq_info++;
                             break;
                          }
                      }
                  }
                  EXPECT_EQ(decoded_info.size(), count_eq_info);
#endif

                  return; // done
            }
        }
        std::cerr
            << "Not found results for '" << name_current_image
            << "' image in config file:" << dataset_config << std::endl
            << "Re-run tests with enabled UPDATE_QRCODE_TEST_DATA macro to update test data."
            << std::endl;
    }
}

INSTANTIATE_TEST_CASE_P(/**/, Objdetect_QRCode, testing::ValuesIn(qrcode_images_name));
INSTANTIATE_TEST_CASE_P(/**/, Objdetect_QRCode_Close, testing::ValuesIn(qrcode_images_close));
INSTANTIATE_TEST_CASE_P(/**/, Objdetect_QRCode_Monitor, testing::ValuesIn(qrcode_images_monitor));
INSTANTIATE_TEST_CASE_P(/**/, Objdetect_QRCode_Curved, testing::ValuesIn(qrcode_images_curved));
INSTANTIATE_TEST_CASE_P(/**/, Objdetect_QRCode_Multi, testing::ValuesIn(qrcode_images_multiple));

TEST(Objdetect_QRCode_decodeMulti, decode_regression_16491)
{
#ifdef HAVE_QUIRC
    Mat zero_image = Mat::zeros(256, 256, CV_8UC1);
    Point corners_[] = {Point(16, 16), Point(128, 16), Point(128, 128), Point(16, 128),
                        Point(16, 16), Point(128, 16), Point(128, 128), Point(16, 128)};
    std::vector<Point> vec_corners;
    int array_size = 8;
    vec_corners.assign(corners_, corners_ + array_size);
    std::vector<cv::String> decoded_info;
    std::vector<Mat> straight_barcode;
    QRCodeDetector vec_qrcode;
    EXPECT_NO_THROW(vec_qrcode.decodeMulti(zero_image, vec_corners, decoded_info, straight_barcode));

    Mat mat_corners(2, 4, CV_32SC2, (void*)&vec_corners[0]);
    QRCodeDetector mat_qrcode;
    EXPECT_NO_THROW(mat_qrcode.decodeMulti(zero_image, mat_corners, decoded_info, straight_barcode));
#endif
}

TEST(Objdetect_QRCode_detectMulti, detect_regression_16961)
{
    const std::string name_current_image = "9_qrcodes.jpg";
    const std::string root = "qrcode/multiple/";

    std::string image_path = findDataFile(root + name_current_image);
    Mat src = imread(image_path);
    ASSERT_FALSE(src.empty()) << "Can't read image: " << image_path;

    QRCodeDetector qrcode;
    std::vector<Point> corners;
    EXPECT_TRUE(qrcode.detectMulti(src, corners));
    ASSERT_FALSE(corners.empty());
    size_t expect_corners_size = 36;
    EXPECT_EQ(corners.size(), expect_corners_size);
}

TEST(Objdetect_QRCode_decodeMulti, check_output_parameters_type_19363)
{
    const std::string name_current_image = "9_qrcodes.jpg";
    const std::string root = "qrcode/multiple/";

    std::string image_path = findDataFile(root + name_current_image);
    Mat src = imread(image_path);
    ASSERT_FALSE(src.empty()) << "Can't read image: " << image_path;
#ifdef HAVE_QUIRC
    QRCodeDetector qrcode;
    std::vector<Point> corners;
    std::vector<cv::String> decoded_info;
#if 0  // FIXIT: OutputArray::create() type check
    std::vector<Mat2b> straight_barcode_nchannels;
    EXPECT_ANY_THROW(qrcode.detectAndDecodeMulti(src, decoded_info, corners, straight_barcode_nchannels));
#endif

    int expected_barcode_type = CV_8UC1;
    std::vector<Mat1b> straight_barcode;
    EXPECT_TRUE(qrcode.detectAndDecodeMulti(src, decoded_info, corners, straight_barcode));
    ASSERT_FALSE(corners.empty());
    for(size_t i = 0; i < straight_barcode.size(); i++)
        EXPECT_EQ(expected_barcode_type, straight_barcode[i].type());
#endif
}

TEST(Objdetect_QRCode_detect, detect_regression_20882)
{
    const std::string name_current_image = "qrcode_near_the_end.jpg";
    const std::string root = "qrcode/";

    std::string image_path = findDataFile(root + name_current_image);
    Mat src = imread(image_path);
    ASSERT_FALSE(src.empty()) << "Can't read image: " << image_path;

    QRCodeDetector qrcode;
    std::vector<Point> corners;
    Mat straight_barcode;
    cv::String decoded_info;
    EXPECT_TRUE(qrcode.detect(src, corners));
    EXPECT_TRUE(!corners.empty());
#ifdef HAVE_QUIRC
    EXPECT_NO_THROW(qrcode.decode(src, corners, straight_barcode));
#endif
}

TEST(Objdetect_QRCode_basic, not_found_qrcode)
{
    std::vector<Point> corners;
    Mat straight_barcode;
    std::string decoded_info;
    Mat zero_image = Mat::zeros(256, 256, CV_8UC1);
    QRCodeDetector qrcode;
    EXPECT_FALSE(qrcode.detect(zero_image, corners));
#ifdef HAVE_QUIRC
    corners = std::vector<Point>(4);
    EXPECT_ANY_THROW(qrcode.decode(zero_image, corners, straight_barcode));
#endif
}

TEST(Objdetect_QRCode_detect, detect_regression_21287)
{
    const std::string name_current_image = "issue_21287.png";
    const std::string root = "qrcode/";

    std::string image_path = findDataFile(root + name_current_image);
    Mat src = imread(image_path);
    ASSERT_FALSE(src.empty()) << "Can't read image: " << image_path;

    QRCodeDetector qrcode;
    std::vector<Point> corners;
    Mat straight_barcode;
    cv::String decoded_info;
    EXPECT_TRUE(qrcode.detect(src, corners));
    EXPECT_TRUE(!corners.empty());
#ifdef HAVE_QUIRC
    EXPECT_NO_THROW(qrcode.decode(src, corners, straight_barcode));
#endif
}

// @author Kumataro, https://github.com/Kumataro
TEST(Objdetect_QRCode_decode, decode_regression_21929)
{
    const cv::String expect_msg = "OpenCV";
    Mat qrImg;
    QRCodeEncoder::Params params;
    params.version = 8; // 49x49
    Ptr<QRCodeEncoder> qrcode_enc = cv::QRCodeEncoder::create(params);;
    qrcode_enc->encode(expect_msg, qrImg);

    Mat src;
    cv::resize(qrImg, src, Size(200,200), 1.0, 1.0, INTER_NEAREST);

    QRCodeDetector qrcode;
    std::vector<Point> corners;
    Mat straight_barcode;

    EXPECT_TRUE(qrcode.detect(src, corners));
    EXPECT_TRUE(!corners.empty());
#ifdef HAVE_QUIRC
    cv::String decoded_msg;
    EXPECT_NO_THROW(decoded_msg = qrcode.decode(src, corners, straight_barcode));
    ASSERT_FALSE(straight_barcode.empty()) << "Can't decode qrimage.";
    EXPECT_EQ(expect_msg, decoded_msg);
#endif
}

TEST(Objdetect_QRCode_decode, decode_regression_version_25)
{
    const cv::String expect_msg = "OpenCV";
    Mat qrImg;
    QRCodeEncoder::Params params;
    params.version = 25; // 117x117
    Ptr<QRCodeEncoder> qrcode_enc = cv::QRCodeEncoder::create(params);;
    qrcode_enc->encode(expect_msg, qrImg);

    Mat src;
    cv::resize(qrImg, src, qrImg.size()*3, 1.0, 1.0, INTER_NEAREST);

    QRCodeDetector qrcode;
    std::vector<Point> corners;
    Mat straight_barcode;

    EXPECT_TRUE(qrcode.detect(src, corners));
    EXPECT_TRUE(!corners.empty());
#ifdef HAVE_QUIRC
    cv::String decoded_msg;
    EXPECT_NO_THROW(decoded_msg = qrcode.decode(src, corners, straight_barcode));
    ASSERT_FALSE(straight_barcode.empty()) << "Can't decode qrimage.";
    EXPECT_EQ(expect_msg, decoded_msg);
#endif
}

#endif // UPDATE_QRCODE_TEST_DATA

}} // namespace
