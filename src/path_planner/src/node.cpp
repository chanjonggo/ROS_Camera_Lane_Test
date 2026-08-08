#include unordered_map

struct Node
{
    int id;
    double x;
    double y;
};

struct Edge
{
    int to;
    double cost;
};

std::unordered_map<int, Node> nodes;
std::unordered_map<int, std::vector<Edge>> graph;