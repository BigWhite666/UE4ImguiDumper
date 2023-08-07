#include <iostream>

extern "C"
{
#include "libavformat/avformat.h"
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libavutil/samplefmt.h>
#include <libavcodec/avcodec.h>
};

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/opencv.hpp>




void AVFrame2Img(AVFrame *pFrame, cv::Mat &img);

void Yuv420p2Rgb32(const uchar *yuvBuffer_in, const uchar *rgbBuffer_out, int width, int height);

using namespace std;
using namespace cv;

int main1(int argc, char *argv[]) {
    AVFormatContext *ifmt_ctx = NULL;
    AVPacket pkt;
    AVFrame *pframe = NULL;
    int ret, i;
    int videoindex = -1;

    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;

    const char *in_filename = "rtmp://58.200.131.2:1935/livetv/hunantv";   //芒果台rtmp地址
    const char *out_filename_v = "test.h264";   //Output file URL
    //Register
    av_register_all();
    //Network
    avformat_network_init();
    //Input
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        printf("Could not open input file.");
        return -1;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        printf("Failed to retrieve input stream information");
        return -1;
    }

    videoindex = -1;
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
        }
    }
    //Find H.264 Decoder
    pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (pCodec == NULL) {
        printf("Couldn't find Codec.\n");
        return -1;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("Couldn't open codec.\n");
        return -1;
    }

    pframe = av_frame_alloc();
    if (!pframe) {
        printf("Could not allocate video frame\n");
        exit(1);
    }

    FILE *fp_video = fopen(out_filename_v, "wb+"); //用于保存H.264

    cv::Mat image_test;

    AVBitStreamFilterContext *h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");

    while (av_read_frame(ifmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == videoindex) {

            av_bitstream_filter_filter(h264bsfc, ifmt_ctx->streams[videoindex]->codec, NULL, &pkt.data, &pkt.size,
                                       pkt.data, pkt.size, 0);

            printf("Write Video Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);

            //保存为h.264 该函数用于测试
            //fwrite(pkt.data, 1, pkt.size, fp_video);

            // Decode AVPacket
            if (pkt.size) {
                ret = avcodec_send_packet(pCodecCtx, &pkt);
                if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    std::cout << "avcodec_send_packet: " << ret << std::endl;
                    continue;
                }
                //Get AVframe
                ret = avcodec_receive_frame(pCodecCtx, pframe);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    std::cout << "avcodec_receive_frame: " << ret << std::endl;
                    continue;
                }
                //AVframe to rgb
                AVFrame2Img(pframe, image_test);
            }

        }
        //Free AvPacket
        av_packet_unref(&pkt);
    }
    //Close filter
    av_bitstream_filter_close(h264bsfc);
    fclose(fp_video);
    avformat_close_input(&ifmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF) {
        printf("Error occurred.\n");
        return -1;
    }
    return 0;
}

void Yuv420p2Rgb32(const uchar *yuvBuffer_in, const uchar *rgbBuffer_out, int width, int height) {
    uchar *yuvBuffer = (uchar *) yuvBuffer_in;
    uchar *rgb32Buffer = (uchar *) rgbBuffer_out;

    int channels = 3;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width + x;

            int indexY = y * width + x;
            int indexU = width * height + y / 2 * width / 2 + x / 2;
            int indexV = width * height + width * height / 4 + y / 2 * width / 2 + x / 2;

            uchar Y = yuvBuffer[indexY];
            uchar U = yuvBuffer[indexU];
            uchar V = yuvBuffer[indexV];

            int R = Y + 1.402 * (V - 128);
            int G = Y - 0.34413 * (U - 128) - 0.71414 * (V - 128);
            int B = Y + 1.772 * (U - 128);
            R = (R < 0) ? 0 : R;
            G = (G < 0) ? 0 : G;
            B = (B < 0) ? 0 : B;
            R = (R > 255) ? 255 : R;
            G = (G > 255) ? 255 : G;
            B = (B > 255) ? 255 : B;

            rgb32Buffer[(y * width + x) * channels + 2] = uchar(R);
            rgb32Buffer[(y * width + x) * channels + 1] = uchar(G);
            rgb32Buffer[(y * width + x) * channels + 0] = uchar(B);
        }
    }
}

void AVFrame2Img(AVFrame *pFrame, cv::Mat &img) {
    int frameHeight = pFrame->height;
    int frameWidth = pFrame->width;
    int channels = 3;
    //输出图像分配内存
    img = cv::Mat::zeros(frameHeight, frameWidth, CV_8UC3);
    Mat output = cv::Mat::zeros(frameHeight, frameWidth, CV_8U);

    //创建保存yuv数据的buffer
    uchar *pDecodedBuffer = (uchar *) malloc(frameHeight * frameWidth * sizeof(uchar) * channels);

    //从AVFrame中获取yuv420p数据，并保存到buffer
    int i, j, k;
    //拷贝y分量
    for (i = 0; i < frameHeight; i++) {
        memcpy(pDecodedBuffer + frameWidth * i,
               pFrame->data[0] + pFrame->linesize[0] * i,
               frameWidth);
    }
    //拷贝u分量
    for (j = 0; j < frameHeight / 2; j++) {
        memcpy(pDecodedBuffer + frameWidth * i + frameWidth / 2 * j,
               pFrame->data[1] + pFrame->linesize[1] * j,
               frameWidth / 2);
    }
    //拷贝v分量
    for (k = 0; k < frameHeight / 2; k++) {
        memcpy(pDecodedBuffer + frameWidth * i + frameWidth / 2 * j + frameWidth / 2 * k,
               pFrame->data[2] + pFrame->linesize[2] * k,
               frameWidth / 2);
    }

    //将buffer中的yuv420p数据转换为RGB;
    Yuv420p2Rgb32(pDecodedBuffer, img.data, frameWidth, frameHeight);

    //简单处理，这里用了canny来进行二值化
    cvtColor(img, output, COLOR_RGB2GRAY);
    waitKey(2);
    Canny(img, output, 50, 50 * 2);
    waitKey(2);
    imshow("test", output);
    waitKey(10);
    // 测试函数
    // imwrite("test.jpg",img);
    //释放buffer
    free(pDecodedBuffer);
    img.release();
    output.release();
}