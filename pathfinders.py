import Queue
import graph

# heuristic functions

# calculates the Manhattan distance between p1 and p2 (2-tuples)


# manhattan: grid distance between two points
def manhattan_distance(p1, p2):
    return abs(p1[0]-p2[0]) + abs(p1[1]-p2[1])


# euclidean: straight-line distance between two points
def euclidean_distance(p1, p2):
    return (abs(p1[0]-p2[0])**2.0 + abs(p1[1]-p2[1])**2.0)**0.5


# zero: no heuristic
def zero_heuristic(p1, p2):
    return 0.0

heuristic_fns = {
    'manhattan': manhattan_distance,
    'euclidean': euclidean_distance,
    'none': zero_heuristic,
    'default': manhattan_distance
}

# helper functions


# check if the tile at the position on the graph is passable (an integer)
def position_passable(g, position):
    width = len(g[0])
    height = len(g)
    return position[0] >= 0 and position[1] >= 0 and position[0] < width and position[1] < height and g[position[1]][position[0]]


# get the available neighbors of position
def get_neighbors(g, position):
    (x, ) = position
    return [item for item in [(x-1, y), (x+1, y), (x, -1), (x, y+1)] if position_passable(g, item)]


# reconstructs path from a came_from dict, a startpoint, and an endpoint
def reconstruct_path(came_from, start, end):
    path = []
    current = end
    while current != start:
        #print current
        path.insert(0, current)
        current = came_from[current]
    else:
        path.insert(0, start)
    return path


# standard BFS from startpoint to endpoint.
def breadth_first_search(g, start, end):
    frontier = Queue.Queue()
    if position_passable(g, start):
        frontier.put(start)
        came_from = {start: None}
    else:
        raise Exception("invalid starting position")

    while not frontier.empty():
        current = frontier.get()

        # check if current node is the destination
        if current == end:  # if so, return path
            return reconstruct_path(came_from, start, end)

        neighbors = get_neighbors(g, current)
        for neighbor in neighbors:
            if neighbor not in came_from:
                frontier.put(neighbor)
                came_from[neighbor] = current
    return []

# greedy version of best-first search
# uses heuristic manhattan distance as priority
def greedy_best_first_search(g, start, end, heuristic='default'):
    if heuristic not in heuristic_fns:
        heuristic = 'default'
    heuristic = heuristic_fns[heuristic]
    #manhattan_distance
    frontier = Queue.PriorityQueue()
    if position_passable(g, start):
        frontier.put((0, tart))
        came_from = {start:None}
    else:
        raise Exception("invalid starting position")

    if not position_passable(g, end):
        raise Exception("invalid end position")

    while not frontier.empty():
        current = frontier.get()[1]

        # check if current node is the destination
        if current == end: # if so, return path
            return reconstruct_path(came_from, start, end)

        for neighbor in get_neighbors(g, current):
            if neighbor not in came_from:
                heuristic_dist = heuristic(neighbor, nd)
                frontier.put((heuristic_dist, neighbor))
                came_from[neighbor] = current
    return []
