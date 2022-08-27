#pragma once

#include "../sausage.h"

using namespace std;

struct MaterialTexNames {
  string diffuse;
  string normal;
  string specular;
  string height;
  string metal;
  string ao;
  string opacity;
  inline size_t Hash() {
    return hash<string>{}(string(diffuse)
                              .append(normal)
                              .append(specular)
                              .append(height)
                              .append(metal)
                              .append(ao)
                              .append(opacity));
  }
};

enum TextureType {
  Diffuse,   // sampler2DArray   index 0
  Normal,    //                        1
  Specular,  //                        2 roughness in PBR
  AO,        //                        3
  Height,    //                        4 PBR
  Metal,     //                        5 PBR
             // Opacity, //                        6 use Diffuse alpha
};
