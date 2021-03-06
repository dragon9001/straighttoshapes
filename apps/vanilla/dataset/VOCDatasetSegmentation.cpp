/**
 * vanilla: VOCDatasetSegmentation.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "../Util.h"
#include "VOCDatasetSegmentation.h"
#include "VOCSegmentationAnnotation.h"

#include "../DetectionUtil.h"

#include <boost/filesystem.hpp>
#include <boost/assign/list_of.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <tvgutil/filesystem/FilesystemUtil.h>
#include <tvgutil/timing/Timer.h>
using namespace tvgutil;

//#################### CONSTRUCTORS ####################

VOCDatasetSegmentation::VOCDatasetSegmentation(const std::string& rootDir)
: VOCDataset(rootDir)
{
  std::list<std::string> expectedPaths = get_split_files(VOC_ALLYEARS, VOC_SEGMENTATION, VOC_ALLSPLITS);
  std::list<std::string> missingPaths;
  missingPaths.splice(missingPaths.end(), FilesystemUtil::get_missing_paths(expectedPaths));
  output_missing_paths(missingPaths);

  initialise_annotation();
}

//#################### PUBLIC MEMBER FUNCTIONS ####################

std::string VOCDatasetSegmentation::get_competition_code(VOCSplit vocSplit, bool seenNonVOCData) const
{
  return "comp5_seg_" + get_split_name(vocSplit);
}

std::vector<std::string> VOCDatasetSegmentation::get_image_paths(VOCYear year, VOCSplit split, VOCImageType imageType, const boost::optional<size_t>& maxPathCount, std::vector<std::string> categories) const
{
  std::list<std::string> splitFiles = get_split_files(year, VOC_SEGMENTATION, split);
  std::vector<std::string> imagePaths = construct_image_paths_from_split_files(splitFiles, imageType);
  if(maxPathCount) truncate_image_paths(imagePaths, *maxPathCount);
  return imagePaths;
}

void VOCDatasetSegmentation::visualise_annotation(const std::string& imagePath) const
{
  cv::Mat3b im = cv::imread(imagePath, CV_LOAD_IMAGE_COLOR);
  std::string imageName = (boost::filesystem::path(imagePath)).stem().string();
  boost::optional<VOCAnnotation_CPtr> segannotation = optionally_get_annotation_from_name(imageName);
  if(segannotation)
  {
    VOCSegmentationAnnotation_CPtr annotation = boost::dynamic_pointer_cast<const VOCSegmentationAnnotation>(*segannotation);
    const size_t counter = 2;
    std::string classWindowName = imageName + " class annotation";
    cv::Mat3b categorySegmentation = annotation->load_class_annotation();
    cv::namedWindow(classWindowName, cv::WINDOW_NORMAL);
    cv::moveWindow(classWindowName, counter*im.cols, 0);
    cv::imshow(classWindowName, categorySegmentation);

    std::string objectWindowName = imageName + " object annotation";
    cv::Mat3b objectSegmentation = annotation->load_object_annotation();
    cv::namedWindow(objectWindowName, cv::WINDOW_NORMAL);
    cv::moveWindow(objectWindowName, (counter+1)*im.cols, 0);
    cv::imshow(objectWindowName, objectSegmentation);

    std::string shapeDetectionWindowName = imageName + " overlayed shape annotation";
    Detections groundTruth = get_detections_from_image_path(imagePath);
    cv::namedWindow(shapeDetectionWindowName, cv::WINDOW_NORMAL);
    cv::moveWindow(shapeDetectionWindowName, (counter+2)*im.cols, 0);
    cv::Mat3b shapeDetectionDisplayImage = DetectionUtil::overlay_detections(im, groundTruth, 1, get_category_names(), get_palette());
    cv::imshow(shapeDetectionWindowName, shapeDetectionDisplayImage);

    std::string maskWindow = imageName + " shape annotation";
    cv::namedWindow(maskWindow, cv::WINDOW_NORMAL);
    cv::moveWindow(maskWindow, (counter+3)*im.cols, 0);
    cv::Mat3b maskImage = cv::Mat::zeros(im.size(), CV_8UC3);
    maskImage = DetectionUtil::overlay_detections(maskImage, groundTruth, 1, get_category_names(), get_palette());
    cv::imshow(maskWindow, maskImage);
  }
}

//#################### PRIVATE MEMBER FUNCTIONS ####################

void VOCDatasetSegmentation::initialise_annotation()
{
  VOCAnnotation::set_category_count(m_categories.size());

  std::vector<std::string> imagePaths = get_image_paths(VOC_ALLYEARS, VOC_TRAINVAL, VOC_JPEG);
  for(size_t i = 0, size = imagePaths.size(); i < size; ++i)
  {
    boost::filesystem::path imagePath(imagePaths[i]);
    std::string imageName = imagePath.stem().string();
    std::string segmentationClassAnnotationPath = imagePath.parent_path().parent_path().string() + "/SegmentationClass/" + imageName + ".png";
    std::string segmentationObjectAnnotationPath = imagePath.parent_path().parent_path().string() + "/SegmentationObject/" + imageName + ".png";
    VOCAnnotation_Ptr annotation(new VOCSegmentationAnnotation(segmentationClassAnnotationPath, segmentationObjectAnnotationPath));
    m_imageNameToAnnotation.insert(std::make_pair(imageName,annotation));
  }
}

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const VOCDatasetSegmentation& d)
{
  os << "Segmentation Dataset\n";
  return os;
}
