

#include <cstdint>
#include <cstdio>
#include <functional>
#include <ranges>
#include <stack>
#include <string>

using x_size_t = int64_t;
using y_size_t = int64_t;
using max_board_size_t = int64_t;
std::string world_print_header = "#Life 1.06";

struct Coordinates
{
    x_size_t x;
    y_size_t y;

    Coordinates() = delete;
    Coordinates(x_size_t in_x, y_size_t in_y) : x(in_x), y(in_y){}
};

struct Cell
{
    Coordinates coordinates_;

    Cell() = delete;
    Cell(Coordinates coordinates) : coordinates_(coordinates) {}
};

enum Cardinal
{
    y_up = -1,
    y_down = 1,
    x_left = -1,
    x_right = 1
};

struct Hash_Cells
{
    size_t operator()(const Coordinates p) const
    {
        size_t xHash = std::hash<int64_t>()(p.x);
        size_t yHash = std::hash<int64_t>()(p.y) << 1;
        return xHash ^ yHash;
    }
};

namespace std
{
    template<> struct equal_to<Coordinates>
    {
        using argument_type = Coordinates;
        using result_type = bool;
        constexpr bool operator()(const Coordinates& lhs, const Coordinates& rhs) const
        { return (lhs.x == rhs.x && lhs.y == rhs.y); }
    };
};

class World
{
    std::unordered_map<Coordinates, std::shared_ptr<Cell>, Hash_Cells> live_cells_;
    std::stack<std::shared_ptr<Cell>> cells_to_cull;
    std::stack<std::shared_ptr<Cell>> cells_to_birth;

public:

    World() = delete;

    explicit World(std::unordered_map<Coordinates, std::shared_ptr<Cell>, Hash_Cells> initial_live_cells)
    { live_cells_.merge(initial_live_cells); }

    void PrintCells()
    {
        printf("%s \n", world_print_header.c_str());
        for (auto it = live_cells_.begin(); it != live_cells_.end(); ++it)
        { printf("%lld %lld \n", it->second->coordinates_.x, it->second->coordinates_.y); }
    }

    void EvaluateWorld()
    {
        for (const auto& val : live_cells_ | std::views::values)
        {
            if (!WillCellSurvive(val))
            { cells_to_cull.push(val); }
        }
    }

    void UpdateWorld()
    {
        while (!cells_to_cull.empty())
        {
            std::shared_ptr<Cell> culled_cell = cells_to_cull.top();
            cells_to_cull.pop();
            live_cells_.erase(culled_cell->coordinates_);
        }

        while (!cells_to_birth.empty())
        {
            std::shared_ptr<Cell> cell_reborn = cells_to_birth.top();
            cells_to_birth.pop();
            live_cells_.try_emplace(cell_reborn->coordinates_, cell_reborn);
        }
    }
    
private:

    // TODO refactor to break down to function calls
    /** NOTE: sites will have the y axis decrement as it moves upwards */
    bool WillCellSurvive(std::shared_ptr<Cell> cell)
    {
        uint8_t live_neighbors = 0;
    
        /** up */
        {
            y_size_t up_y = cell->coordinates_.y + y_up;
            if (up_y >= INT64_MIN)
            {
                live_cells_.contains(Coordinates(cell->coordinates_.x, up_y)) ?
                    ++live_neighbors :
                    ResurrectCell(Coordinates(cell->coordinates_.x, up_y));
            }
        }

        /** down */
        {
            y_size_t down_y = cell->coordinates_.y + y_down;
            if (down_y <= INT64_MAX)
            {
                live_cells_.contains(Coordinates(cell->coordinates_.x, down_y)) ?
                    ++live_neighbors :
                    ResurrectCell(Coordinates(cell->coordinates_.x, down_y));
            }
        }

        /** right */
        {
            x_size_t right_x = cell->coordinates_.x + x_right;
            if (right_x <= INT64_MAX)
            {
                live_cells_.contains(Coordinates(right_x, cell->coordinates_.y)) ?
                    ++live_neighbors :
                    ResurrectCell(Coordinates(right_x, cell->coordinates_.y));
            }
            
        }

        /** left */
        {
            x_size_t left_x = cell->coordinates_.x + x_left;
            if (left_x >= INT64_MIN)
            {
                live_cells_.contains(Coordinates(left_x, cell->coordinates_.y)) ?
                    ++live_neighbors :
                    ResurrectCell(Coordinates(left_x, cell->coordinates_.y));
            }
        }

        /** up right */
        {
            x_size_t right_x = cell->coordinates_.x + x_right;
            y_size_t up_y = cell->coordinates_.y + y_up;
            if (right_x <= INT64_MAX && up_y >= INT64_MIN)
            {
                live_cells_.contains(Coordinates(right_x, up_y)) ?
                    ++live_neighbors :
                    ResurrectCell(Coordinates(right_x, up_y));
            }
        }

        /** down right */
        {
            x_size_t right_x = cell->coordinates_.x + x_right;
            y_size_t down_y = cell->coordinates_.y + y_down;
            if (right_x <= INT64_MAX && down_y <= INT64_MAX)
            {
                live_cells_.contains(Coordinates(right_x, down_y)) ?
                    ++live_neighbors :
                    ResurrectCell(Coordinates(right_x, down_y));
            }
        }

        /** up left */
        {
            x_size_t left_x = cell->coordinates_.x + x_left;
            y_size_t up_y = cell->coordinates_.y + y_up;
            if (left_x >= INT64_MIN && up_y >= INT64_MIN)
            {
                live_cells_.contains(Coordinates(left_x, up_y)) ?
                    ++live_neighbors :
                    ResurrectCell(Coordinates(left_x, up_y));
            }
        }

        /** down left */
        {
            x_size_t left_x = cell->coordinates_.x + x_left;
            y_size_t down_y = cell->coordinates_.y + y_down;
            if (left_x >= INT64_MIN && down_y <= INT64_MAX)
            {
                live_cells_.contains(Coordinates(left_x, down_y)) ?
                    ++live_neighbors :
                    ResurrectCell(Coordinates(left_x, down_y));
            }
        }

        if (live_neighbors == 2 || live_neighbors == 3)
        { return true; }

        return false;
    }

    // TODO dedupe this unholiness with WillCellSurvive
    bool ResurrectCell(Coordinates coordinates)
    {
        uint8_t live_neighbors = 0;
        
        /** up */
        {
            y_size_t up_y = coordinates.y + y_up;
            if (up_y >= INT64_MIN)
            { live_neighbors += live_cells_.contains(Coordinates(coordinates.x, up_y)); }
        }

        /** down */
        {
            y_size_t down_y = coordinates.y + y_down;
            if (down_y <= INT64_MAX)
            { live_neighbors += live_cells_.contains(Coordinates(coordinates.x, down_y)); }
        }

        /** left */
        {
            x_size_t left_x = coordinates.x + x_left;
            if (left_x >= INT64_MIN)
            { live_neighbors += live_cells_.contains(Coordinates(left_x, coordinates.y)); }
        }

        /** right */
        {
            x_size_t right_x = coordinates.x + x_right;
            if (right_x <= INT64_MAX)
            { live_neighbors += live_cells_.contains(Coordinates(right_x, coordinates.y)); }
        }

        /** up right */
        {
            x_size_t right_x = coordinates.x + x_right;
            y_size_t up_y = coordinates.y + y_up;
            if (right_x <= INT64_MAX && up_y >= INT64_MIN)
            { live_neighbors += live_cells_.contains(Coordinates(right_x, up_y)); }
        }

        /** down right */
        {
            x_size_t right_x = coordinates.x + x_right;
            y_size_t down_y = coordinates.y + y_down;
            if (right_x <= INT64_MAX && down_y <= INT64_MAX)
            { live_neighbors += live_cells_.contains(Coordinates(right_x, down_y)); }
        }

        /** up left */
        {
            x_size_t left_x = coordinates.x + x_left;
            y_size_t up_y = coordinates.y + y_up;
            if (left_x >= INT64_MIN && up_y >= INT64_MIN)
            { live_neighbors += live_cells_.contains(Coordinates(left_x, up_y)); }
        }

        /** down left */
        {
            x_size_t left_x = coordinates.x + x_left;
            y_size_t down_y = coordinates.y + y_down;
            if (left_x >= INT64_MIN && down_y <= INT64_MAX)
            { live_neighbors += live_cells_.contains(Coordinates(left_x, down_y)); }
        }

        if (live_neighbors == 3)
        {
            auto cell_reborn = std::make_shared<Cell>(coordinates);
            cells_to_birth.push(cell_reborn);
            return true;
        }
        return false;
    }
};

int main(int argc, char* argv[])
{
    
    // std::unordered_map<Coordinates, std::shared_ptr<Cell>, Hash_Cells> max_bounds_test = {
    //     { Coordinates(INT64_MIN,INT64_MIN), std::make_shared<Cell>(Coordinates(INT64_MIN,INT64_MIN)) },
    //     { Coordinates(INT64_MIN,INT64_MIN+1), std::make_shared<Cell>(Coordinates(INT64_MIN,INT64_MIN+1)) },
    //     { Coordinates(INT64_MIN+1,INT64_MIN), std::make_shared<Cell>(Coordinates(INT64_MIN+1,INT64_MIN)) }
    // };

    std::unordered_map<Coordinates, std::shared_ptr<Cell>, Hash_Cells> glider = {
        { Coordinates(0,-1), std::make_shared<Cell>(Coordinates(0,-1)) },
        { Coordinates(1,0), std::make_shared<Cell>(Coordinates(1,0)) },
        { Coordinates(-1,1), std::make_shared<Cell>(Coordinates(-1,1)) },
        { Coordinates(0,1), std::make_shared<Cell>(Coordinates(0,1)) },
        { Coordinates(1,1), std::make_shared<Cell>(Coordinates(1,1)) }
    };
    /*
     * after 1 generation - check https://copy.sh/life/
    * #Life 1.06
    * 1 0
    * 0 1
    * 1 1
    * 0 2
    * -1 0
    */
    
    auto world = new World(glider);
    size_t world_generations = 100;
    while(world_generations > 0)
    {
        --world_generations;
        world->EvaluateWorld();
        world->UpdateWorld();
    }
    world->PrintCells();
    printf(" \n");
}
