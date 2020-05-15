#pragma once

#include <chrono>
#include <type_traits>

namespace ch = std::chrono;

namespace utils
{
    template<typename resolution_t = std::chrono::milliseconds, typename duration_t = double>
    class ExecutionTimer
    {
        using clock_t = std::conditional_t<ch::high_resolution_clock::is_steady,
            ch::high_resolution_clock,
            ch::steady_clock>;

        using time_pt = clock_t::time_point;

      public:
        ExecutionTimer()
        {
            start();
        }

        void start()
        {
            start_ = lastckpt_ = clock_t::now();
        }

        duration_t check()
        {
            checkpoint_ = clock_t::now();
            auto duration = elapsed(lastckpt_, checkpoint_);
            lastckpt_ = checkpoint_;
            return duration;
        }

        duration_t stop()
        {
            end_ = clock_t::now();
            return time();
        }

        duration_t time() const
        {
            return elapsed(start_, end_);
        }

        bool isRunning() const
        {
            return !(end_ > resolution_t::zero());
        }

      private:
        inline duration_t elapsed(const time_pt& s, const time_pt& e) const
        {
            return ch::duration_cast<resolution_t>(e - s).count();
        }

        time_pt start_;
        time_pt lastckpt_;
        time_pt checkpoint_;
        time_pt end_;
    };

} // namespace utils
