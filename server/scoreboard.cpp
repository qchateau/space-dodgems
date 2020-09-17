#include "scoreboard.h"
#include "player.h"

namespace sd {

const score_t& scoreboard_t::lowest_score() const
{
    return scores_.back();
}

void scoreboard_t::push_score(const score_t& score)
{
    if (!scores_.empty() && score.score <= lowest_score().score) {
        return;
    }

    bool inserted = false;
    for (auto it = begin(scores_); it < end(scores_); ++it) {
        if (!inserted) {
            if (score.score > it->score) {
                inserted = true;

                if (score.id == it->id) {
                    // just replace and bail out
                    *it = score;
                    break;
                }
                else {
                    // insert, shift the other scores
                    scores_.insert(it, score);
                }
            }
            if (score.id == it->id) {
                // same player already has a better score
                break;
            }
        }
        else {
            if (score.id == it->id) {
                // remove previous highest score
                scores_.erase(it--);
            }
        }
    }

    if (!inserted && scores_.size() < max_size) {
        scores_.push_back(score);
    }

    while (scores_.size() > max_size) {
        scores_.pop_back();
    }
}

void scoreboard_t::push_score(const player_t& player)
{
    if (scores_.empty() || player.score() > lowest_score().score) {
        return push_score({player.id(), player.name(), player.score()});
    }
}

} // sd
