#include "pch.h"
#include "ReelJudge.h"

std::string ReelJudge::Judge(const std::array<std::string, 3>& s)
{
    // 3つ揃い
    if (s[0] == s[1] && s[1] == s[2]) {
        if (s[0] == "7")        return "BIG";
        if (s[0] == "ベル")     return "ベル揃い";
        if (s[0] == "リプレイ") return "リプレイ";
        if (s[0] == "BAR")      return "BAR揃い";
        if (s[0] == "力")       return "力揃い";
    }

    // 2連（例: 左中 or 中右）
    if (s[0] == s[1] || s[1] == s[2]) {
        return s[1] + "2連";
    }

    // バラバラ
    return "ハズレ";
}
