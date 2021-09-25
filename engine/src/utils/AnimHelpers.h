#pragma once

#include "sausage.h"
using namespace std;
using namespace glm;

vec3 _GetBlendAnim(vector<pair<double, vec3>>& frame,
    double current_time_ticks,
    bool is_bone_anim,
    vec3& prev_anim,
    float blend_anims
);
quat _GetBlendAnim(vector<pair<double, quat>>& frame,
    double current_time_ticks,
    bool is_bone_anim,
    quat& prev_anim,
    float blend_anims
);