

/* Work of James65535.github.io */
#include <cstdio>
#include <memory>
#include <stack>
#include <string>
#include <unordered_set>

using x_size_t = int64_t;
using y_size_t = int64_t;

enum Cardinal
{
    Y_UP = -1,
    Y_DOWN = 1,
    X_LEFT = -1,
    X_RIGHT = 1,
    ZERO_VAL = 0
};

/** size is 16bytes */
struct Coordinates
{
    x_size_t x_;
    y_size_t y_;

    Coordinates() = delete;
    Coordinates(x_size_t in_x, y_size_t in_y) : x_(in_x), y_(in_y){}

    Coordinates operator+(const Coordinates& val) const
    { return {this->x_ + val.x_, this->y_ + val.y_}; }

    Coordinates operator+=(const Coordinates& val)
    { return {this->x_ += val.x_, this->y_ += val.y_}; }

    bool operator==(const Coordinates& val) const
    { return this->x_ == val.x_ && this->y_ == val.y_; }

    struct Hash_Coordinates
    {
        size_t operator()(const Coordinates p) const
        {
            const size_t xHash = std::hash<int64_t>()(p.x_);
            const size_t yHash = std::hash<int64_t>()(p.y_) << 1;
            return xHash ^ yHash;
        }
    };

    /** Allows for bounds checking on coordinates whether they are original or can test if incremented with adjusted flag set to true */
    bool WithinBounds(const bool adjusted, Cardinal x_Left = ZERO_VAL, Cardinal x_right = ZERO_VAL, Cardinal y_up = ZERO_VAL, Cardinal y_down = ZERO_VAL) const
    {
        if (adjusted)
        {
            if (x_ + x_Left >= INT64_MIN && x_ + x_right <= INT64_MAX &&
            y_ + y_up >= INT64_MIN && y_ + y_down <= INT64_MAX)
            { return true; }
        }
        else
        {
            if (x_ >= INT64_MIN && x_ <= INT64_MAX &&
            y_ >= INT64_MIN && y_  <= INT64_MAX)
            { return true; }
        }
        return false;
    }

    void Update(const Cardinal x, const Cardinal y)
    { x_ += x; y_ += y; }
};

class World
{
    std::unordered_set<Coordinates, Coordinates::Hash_Coordinates> living_cell_coordinates_;
    std::stack<Coordinates> cells_to_cull_;
    std::stack<Coordinates> cells_to_birth_;
    std::vector<Coordinates> coordinates_to_process_{8, Coordinates(0,0)};

public:

    World() = delete;

    explicit World(std::unordered_set<Coordinates, Coordinates::Hash_Coordinates>& initial_live_cells)
    {
        living_cell_coordinates_.merge(initial_live_cells);
        
        coordinates_to_process_[0].Update(ZERO_VAL, Y_UP); /** check up */
        coordinates_to_process_[1].Update(ZERO_VAL, Y_DOWN); /** check down */
        coordinates_to_process_[2].Update(X_LEFT, ZERO_VAL); /** check left */
        coordinates_to_process_[3].Update(X_RIGHT, ZERO_VAL); /** check right */
        coordinates_to_process_[4].Update(X_RIGHT, Y_UP); /** check right upper */
        coordinates_to_process_[5].Update(X_LEFT, Y_UP); /** check left upper */
        coordinates_to_process_[6].Update(X_RIGHT, Y_DOWN); /** check right down */
        coordinates_to_process_[7].Update(X_LEFT, Y_DOWN); /** check left down */
    }

    void PrintWorld(const std::string& header) const
    {
        printf("%s \n", header.c_str());
        for (const auto& cell : living_cell_coordinates_)
        { printf("%lld %lld \n", cell.x_, cell.y_); }
    }
    
    void EvaluateWorld()
    {
        for (auto cell : living_cell_coordinates_)
        {
            if (cell.WithinBounds(false))
            { CheckNeighborCells(cell); }
        }
    }
    
    void UpdateWorld()
    {
        while (!cells_to_cull_.empty())
        {
            Coordinates culled_cell_coordinates = cells_to_cull_.top();
            cells_to_cull_.pop();
            living_cell_coordinates_.erase(culled_cell_coordinates);
        }
        
        while (!cells_to_birth_.empty())
        {
            Coordinates the_one_reborn_coordinates = cells_to_birth_.top();
            cells_to_birth_.pop();
            living_cell_coordinates_.insert(the_one_reborn_coordinates);
        }
    }
    
protected:
    
    void CheckNeighborCells(Coordinates& source_cell_location)
    {
        uint8_t living_neighbor_count = 0;
        for (const auto& cell_coordinate_offset : coordinates_to_process_)
        {
            const Coordinates cell_coordinates(cell_coordinate_offset + source_cell_location);
            if (cell_coordinates.WithinBounds(true))
            {
                if(living_cell_coordinates_.contains(cell_coordinates))
                { living_neighbor_count++; }
                else
                { CountResurrectedDeadCells(cell_coordinates); }
            }
        }
        
        /** These living cells affect whether source cell lives or dies */
        if (living_neighbor_count < 2 || living_neighbor_count > 3)
        { cells_to_cull_.emplace(source_cell_location); }
    }

    void CountResurrectedDeadCells(const Coordinates& coordinates)
    {
        /** found a dead cell - determine if it can be reborn */
        uint8_t dead_living_neighbor_count = 0;
        for (const auto& coordinate_offset : coordinates_to_process_)
        {
            const Coordinates cell(coordinates + coordinate_offset);
            if (cell.WithinBounds(true) && living_cell_coordinates_.contains(cell))
            { dead_living_neighbor_count++; }
        }
        /** These neighboring dead cell will live again and avoid a liberal arts degree */
        if (dead_living_neighbor_count == 3)
        { cells_to_birth_.push(coordinates); }
    }
};

int main(int argc, char* argv[])
{
    /** Test Values for INT64 min and max values */
    //  std::unordered_set<Coordinates, Coordinates::Hash_Coordinates> max_bounds_test = {
    //     { Coordinates(INT64_MIN,INT64_MIN), std::make_shared<Cell>(Coordinates(INT64_MIN,INT64_MIN)) },
    //     { Coordinates(INT64_MIN,INT64_MIN+1), std::make_shared<Cell>(Coordinates(INT64_MIN,INT64_MIN+1)) },
    //     { Coordinates(INT64_MIN+1,INT64_MIN), std::make_shared<Cell>(Coordinates(INT64_MIN+1,INT64_MIN)) }
    // };
    
    /** Test values to verify standard life automata behaves correctly generation to generation */
    std::unordered_set<Coordinates, Coordinates::Hash_Coordinates> glider = {
        { Coordinates(0,-1) },
        { Coordinates(1,0) },
        { Coordinates(-1,1) },
        { Coordinates(0,1) },
        { Coordinates(1,1) }
    };
    /*
     * Glider after 1 generation - check https://copy.sh/life/
    * #Life 1.06
    * 1 0
    * 0 1
    * 1 1
    * 0 2
    * -1 0
    */
    
    /** game loop */
    auto world = new World(glider);
    size_t world_generations = 1;
    const std::string print_header = "#Life 1.06";
    while(world_generations > 0)
    {
        --world_generations;
        world->EvaluateWorld();
        world->UpdateWorld();
    }
    world->PrintWorld(print_header);
    delete world; /** RAII would take care of this */
}
