//
// Created by chris on 18-8-9.
//

#ifndef OPENVINO_PIPELINE_LIB_REALSENSE_CAMERA_H
#define OPENVINO_PIPELINE_LIB_REALSENSE_CAMERA_H

#include "openvino_service/inputs/base_input.h"

#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>

namespace Input {
class RealSenseCamera : public BaseInputDevice {
 public:
  bool initialize() override;
  bool initialize(int t) override { return true; };
  bool initialize(size_t width, size_t height) override;
  bool read(cv::Mat *frame) override;
  void config() override;
 private:
  rs2::config cfg_;
  rs2::pipeline pipe_;
  bool first_read_ = true;
};
}

#endif //OPENVINO_PIPELINE_LIB_REALSENSE_CAMERA_H
