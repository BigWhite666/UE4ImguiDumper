// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Copyright (C) 2018, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.

#ifndef OPENCV_DNN_VKCOM_CONTEXT_HPP
#define OPENCV_DNN_VKCOM_CONTEXT_HPP

namespace cv { namespace dnn { namespace vkcom {

#ifdef HAVE_VULKAN

class Context
{
public:
    Context();
    ~Context();
};

void createContext();

#endif // HAVE_VULKAN

}}} // namespace cv::dnn::vkcom

#endif // OPENCV_DNN_VKCOM_CONTEXT_HPP
