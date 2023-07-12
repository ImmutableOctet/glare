#include <catch2/catch_test_macros.hpp>

#include <util/history_log.hpp>

#include <array>

#include <cstdint>

TEST_CASE("util::HistoryLog", "[util:history_log]")
{
    SECTION("Empty state")
    {
        auto history = util::HistoryLog<std::int32_t>{};

        REQUIRE(history.empty());
        REQUIRE(history.size() == 0);
        
        REQUIRE(history.has_default_cursor());
        REQUIRE(history.has_live_value_cursor());

        REQUIRE(!history.can_truncate());
        REQUIRE(!history.can_undo());
        REQUIRE(!history.can_redo());
    }

    SECTION("Store")
    {
        auto history = util::HistoryLog<std::int32_t> {};

        REQUIRE(history.empty());
        REQUIRE(!history.can_undo());
        REQUIRE(!history.can_redo());
        REQUIRE(history.has_live_value_cursor());

        history << 100;

        REQUIRE(!history.empty());
        REQUIRE(history.size() == 1);

        REQUIRE(history.has_live_value_cursor());

        REQUIRE(history.can_undo());
        REQUIRE(!history.can_redo());
    }

    SECTION("Clear")
    {
        auto history = util::HistoryLog<std::int32_t> {};

        REQUIRE(history.empty());
        REQUIRE(!history.can_undo());
        REQUIRE(!history.can_redo());
        REQUIRE(history.has_default_cursor());
        REQUIRE(history.has_live_value_cursor());

        history << 10;

        REQUIRE(!history.empty());
        REQUIRE(!history.can_redo());
        REQUIRE(!history.has_default_cursor());
        REQUIRE(history.has_live_value_cursor());
        
        REQUIRE(history.can_undo());

        history.undo();

        REQUIRE(history.can_redo());
        REQUIRE(!history.has_live_value_cursor());

        history.redo();

        REQUIRE(!history.has_default_cursor());
        REQUIRE(history.has_live_value_cursor());

        history.clear();

        REQUIRE(history.empty());
        REQUIRE(!history.can_undo());
        REQUIRE(!history.can_redo());
        REQUIRE(history.has_default_cursor());
        REQUIRE(history.has_live_value_cursor());
    }

    SECTION("Store and undo")
    {
        const auto input_values = std::array
        {
            1, 2, 3, 4
        };

        using T = typename decltype(input_values)::value_type;

        auto history = util::HistoryLog<T> {};

        for (const auto& value : input_values)
        {
            REQUIRE(history.store(value));
        }

        REQUIRE(history.size() == 4); // (history.size() == input_values.size())

        REQUIRE(history.undo());
        REQUIRE(history.undo());

        REQUIRE(history.size() == 4);

        REQUIRE(history.can_truncate());

        REQUIRE(history.truncate());

        REQUIRE(history.size() == 2);
        REQUIRE(history.cursor_points_to_live_value());
        
        const auto value_to_add = 5;

        history.store(value_to_add);

        REQUIRE(history.size() == 3);
        REQUIRE(history.cursor_points_to_live_value());

        REQUIRE((*history.get_snapshot(static_cast<std::size_t>(history.get_cursor()) - static_cast<std::size_t>(1))) == value_to_add);
        REQUIRE((*history.get_snapshot(static_cast<std::size_t>(history.get_cursor()) - static_cast<std::size_t>(2))) == input_values[1]);
        REQUIRE((*history.get_snapshot(static_cast<std::size_t>(history.get_cursor()) - static_cast<std::size_t>(3))) == input_values[0]);
    }

    SECTION("Undo and redo")
    {
        const auto input_values = std::array
        {
            1, 2, 3, 4
        };

        using T = typename decltype(input_values)::value_type;

        auto history = util::HistoryLog<T>{};

        for (const auto& value : input_values)
        {
            REQUIRE(history.store(value));
        }

        REQUIRE(history.size() == 4); // (history.size() == input_values.size())

        REQUIRE(history.can_undo());
        REQUIRE(!history.can_redo());

        const auto first_undo = history.undo();

        REQUIRE(first_undo);
        REQUIRE((*first_undo) == 4);

        REQUIRE(history.can_undo());
        REQUIRE(history.can_redo());

        const auto second_undo = history.undo();

        REQUIRE(second_undo);
        REQUIRE((*second_undo) == 3);
        
        REQUIRE(history.size() == 4);

        REQUIRE(history.can_undo());
        REQUIRE(history.can_redo());

        const auto first_redo = history.redo();

        REQUIRE(first_redo);
        REQUIRE((*first_redo) == input_values[input_values.size() - 2]);

        REQUIRE(history.size() == 4);

        REQUIRE(history.can_undo());
        REQUIRE(history.can_redo());

        const auto second_redo = history.redo();

        REQUIRE(second_redo);
        REQUIRE((*second_redo) == input_values[input_values.size() - 1]);

        REQUIRE(second_redo);

        REQUIRE(history.size() == 4);
        REQUIRE(history.get_cursor() == history.live_value_cursor());

        REQUIRE(history.can_undo());
        REQUIRE(!history.can_redo());

        const auto undo_after_redo = history.undo();

        REQUIRE(undo_after_redo);

        REQUIRE((*undo_after_redo) == input_values[input_values.size() - 1]);

        REQUIRE(history.size() == 4);
        REQUIRE(history.get_cursor() == (history.live_value_cursor() - 1));
        REQUIRE(history.can_undo());
        REQUIRE(history.can_redo());

        REQUIRE(history.redo());
        REQUIRE(!history.can_redo());
        REQUIRE(!history.redo());

        while (history.can_undo())
        {
            REQUIRE(history.undo());
        }

        REQUIRE(!history.can_undo());
        REQUIRE(!history.undo());
    }

    SECTION("Undo, redo and truncate")
    {
        const auto input_values = std::array
        {
            1, 2, 3, 4
        };

        using T = typename decltype(input_values)::value_type;

        auto history = util::HistoryLog<T> {};

        for (const auto& value : input_values)
        {
            REQUIRE(history.store(value));
        }

        REQUIRE(history.size() == 4); // (history.size() == input_values.size())

        REQUIRE(history.undo());
        REQUIRE(history.undo());

        REQUIRE(history.size() == 4);

        REQUIRE(history.can_truncate());

        REQUIRE(history.truncate());

        REQUIRE(history.has_live_value_cursor());
        REQUIRE(history.size() == 2);

        while (history.can_undo())
        {
            REQUIRE(history.undo());
        }

        REQUIRE(history.can_redo());

        REQUIRE(history.redo());

        REQUIRE(history.size() == 2);

        REQUIRE(history.can_truncate());

        REQUIRE(history.truncate());

        REQUIRE(history.has_live_value_cursor());

        REQUIRE(!history.empty());
        REQUIRE(history.size() == 1);
        
        REQUIRE(!history.can_truncate());
        REQUIRE(history.can_undo());
        REQUIRE(!history.can_redo());

        REQUIRE(history.undo());

        const auto active_snapshot = history.get_active_snapshot();

        REQUIRE(active_snapshot);
        REQUIRE((*active_snapshot) == input_values[0]);

        REQUIRE(history.can_truncate());
        REQUIRE(history.truncate());

        REQUIRE(history.size() == 0);
        REQUIRE(history.empty());
        
        REQUIRE(history.has_live_value_cursor());
        
        REQUIRE(!history.can_undo());
        REQUIRE(!history.can_redo());
    }

	SECTION("Stream operators")
	{
        const auto input_values = std::array
        {
            10, 20, 30, 40
        };

        using T = typename decltype(input_values)::value_type;

        auto history = util::HistoryLog<T> {};

        for (const auto& value : input_values)
        {
            history << value;
        }

        REQUIRE(history.size() == input_values.size());

        auto values_processed = static_cast<std::size_t>(0);

        while (history)
        {
            auto value = T {};

            history >> value;

            values_processed++;

            const auto& original_input_value = input_values[input_values.size() - values_processed];

            REQUIRE(value == original_input_value);
        }

        REQUIRE(history.size() == input_values.size());
        
        REQUIRE(history.truncate());

        REQUIRE(history.empty());
        REQUIRE(history.size() == 0);
	}

    SECTION("Undo and redo triggers")
    {
        const auto input_values = std::array
        {
            5, 7, 9, 11
        };

        using T = typename decltype(input_values)::value_type;

        auto history = util::HistoryLog<T> {};

        for (const auto& value : input_values)
        {
            history << value;
        }

        auto test_first_undo = [&history](bool response=true) -> bool
        {
            bool trigger_found = false;

            auto undo_result = history.undo
            (
                [](const auto&) {},

                [response, &history, &trigger_found](auto& instance)
                {
                    REQUIRE((&instance) == (&history));

                    trigger_found = true;

                    return response;
                }
            );

            if (!undo_result)
            {
                return false;
            }

            return trigger_found;
        };

        REQUIRE(history.has_live_value_cursor());
        
        REQUIRE(!test_first_undo(false));

        REQUIRE(history.has_live_value_cursor());

        REQUIRE(test_first_undo());
        REQUIRE(!test_first_undo());

        while (history.can_redo())
        {
            REQUIRE(history.redo());
        }

        REQUIRE(history.has_live_value_cursor());

        REQUIRE(test_first_undo());
        REQUIRE(!test_first_undo());

        REQUIRE(!history.has_live_value_cursor());
        REQUIRE(history.can_redo());

        REQUIRE(history.redo());

        auto test_last_redo = [&history](bool response=true) -> bool
        {
            bool trigger_found = false;

            auto redo_result = history.redo
            (
                [](const auto&) {},

                [response, &history, &trigger_found](const auto& instance)
                {
                    REQUIRE((&instance) == (&history));

                    trigger_found = true;

                    return response;
                }
            );

            if (!redo_result)
            {
                return false;
            }

            return trigger_found;
        };

        REQUIRE(!history.has_live_value_cursor());
        REQUIRE(history.can_redo());

        REQUIRE(!test_last_redo(false));
        REQUIRE(test_last_redo());

        REQUIRE(history.has_live_value_cursor());
        REQUIRE(history.can_undo());
        
        REQUIRE(history.undo());

        REQUIRE(!history.has_live_value_cursor());
        REQUIRE(history.can_undo());

        REQUIRE(history.undo());

        REQUIRE(!history.has_live_value_cursor());
        REQUIRE(history.can_redo());

        REQUIRE(!test_last_redo());

        REQUIRE(!history.has_live_value_cursor());
        REQUIRE(history.can_redo());

        REQUIRE(test_last_redo());
    }
}