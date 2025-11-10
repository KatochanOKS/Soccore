#pragma once
#include <string>
#include <array>

/// <summary>
/// 3本リールの出目（停止図柄）から役を判定するクラス
/// </summary>

class ReelJudge {
public:

    /// <summary>
    /// 出目(左,中,右)から役名を返す
    /// </summary>
    static std::string Judge(const std::array<std::string, 3>& symbols);
};

