#pragma once

#include "sausage.h"
using namespace std;
using namespace glm;

vec3 _GetBlendAnim(vector<pair<double, vec3>>& frame,
    double current_time_ticks,
    bool is_bone_anim,
    vec3& prev_anim,
    float blend_anims
) {
    auto frame_iter = frame.begin();
    while (frame_iter != frame.end()) {
        auto cur = *frame_iter;
        auto next = ++frame_iter;
        bool is_end = next == frame.end();
        if (is_end || (*next).first > current_time_ticks) {
            if (is_end) {
                if (is_bone_anim) {
                    return mix(cur.second, prev_anim, blend_anims);
                }
                else {
                    return cur.second;
                }
            }
            else {
                float blend = (float)(current_time_ticks - cur.first) / ((*next).first - cur.first);
                if (blend < 0) blend = 0;
                if (is_bone_anim) {
                    return mix(mix(cur.second, (*next).second, blend), prev_anim, blend_anims);
                }
                else {
                    return mix(cur.second, (*next).second, blend);
                }
            }
            break;
        }
    }
}
quat _GetBlendAnim(vector<pair<double, quat>>& frame,
    double current_time_ticks,
    bool is_bone_anim,
    quat& prev_anim,
    float blend_anims
) {
    auto frame_iter = frame.begin();
    while (frame_iter != frame.end()) {
        auto cur = *frame_iter;
        auto next = ++frame_iter;
        bool is_end = next == frame.end();
        if (is_end || (*next).first > current_time_ticks) {
            if (is_end) {
                if (is_bone_anim) {
                    return slerp(cur.second, prev_anim, blend_anims);
                }
                else {
                    return cur.second;
                }
            }
            else {
                float blend = (float)(current_time_ticks - cur.first) / ((*next).first - cur.first);
                if (blend < 0) blend = 0;
                if (is_bone_anim) {
                    return slerp(slerp(cur.second, (*next).second, blend), prev_anim, blend_anims);
                }
                else {
                    return slerp(cur.second, (*next).second, blend);
                }
            }
            break;
        }
    }
}