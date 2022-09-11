#include "GLHelpers.h"

// cannot be constexpr because of `va_start`
//size_t GetPadCharsNumToLargestElement(int num, ...) {
//  va_list elements;
//  int total_size = 0;
//  int max_elem_size = 0;
//  va_start(elements, num);
//  for (int i = 0; i < num; i++) {
//    size_t elem = va_arg(elements, size_t);
//    total_size += elem;
//    max_elem_size = max_elem_size < elem ? elem : max_elem_size;
//  }
//  va_end(elements, num);
//
//  return (max_elem_size - total_size % max_elem_size) / sizeof(char);
//}

//constexpr size_t GetPadCharsNumToLargestElement(vector<size_t> sizes) {
//  int total_size = 0;
//  int max_elem_size = 0;
//  for (int i = 0; i < sizes.size(); i++) {
//    size_t elem = sizes[i];
//    total_size += elem;
//    max_elem_size = max_elem_size < elem ? elem : max_elem_size;
//  }
//  return (max_elem_size - total_size % max_elem_size) / sizeof(char);
//}
