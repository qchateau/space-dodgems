#pragma once

#include <string>
#include <vector>

#include "config.h"

namespace sd {

struct score_t {
    player_id_t id;
    std::string name;
    double score;
};

class scoreboard_t {
public:
    static constexpr auto max_size = 10;

    const std::vector<score_t>& scores() const { return scores_; }
    void push_score(const score_t& score);
    void push_score(const player_t& player);

private:
    const score_t& lowest_score() const;

    std::vector<score_t> scores_;
};

} // sd