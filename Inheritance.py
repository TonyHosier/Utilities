import time

# From google code jam:
# http://code.google.com/codejam/contest/1781488/dashboard#s=p0

# This is just an exercise of something constructive to do using python

# Use the same variable names as described in the problem. Normally, more
# intuitive names would be used.

# There are a couple ways of doing this. Basically the graph needs to be
# traversed depth-first, starting with a node and traversing all child nodes.
# If there are no cycles, then every node will have been visited once.

# Also, the traversal needs to start from each node in the graph; There is
# no assumption that node 1 is the 'root' or in fact it's a tree structure
# at all.
# When deciding which node to start with, an optimization done here is
# to ignore all nodes which have previously been visited. On the large
# test, this results in a 700% speedup

####################################################################
# from the node at idx, traverse the graph using a depth-first search
# if any nodes have been visited 2 times or more, then there is more than
# 1 path to that node from the starting node.
def traverse(classList, visited, totalNodes, idx):
    # keep a count of the number of times this node has been visited
    classCount = [0]*totalNodes;
    classQueue = [];

    # put first element into class queue
    classQueue.append(idx);

    # keep adding elements to the classQueue. These will be the children
    # of the current node. Once added, pop the old node off the top.
    # rince and repeat until the queue is empty.
    while (len(classQueue) > 0):
        index = (int)(classQueue[0]);       # get the current node
        classQueue.pop(0);                  # pop it off
        i = classList[index - 1];           # get a list of all current node's children
        for j in i:
            next = (int)(j);
            next0 = next - 1;
            classQueue.append(next);
            classCount[next0] = classCount[next0] + 1;
            visited[next0] = 1;

            # abort once a cycle has been found. Otherwise it could continue indefinitely
            if ((int)(classCount[next - 1]) >= 2):
                return 1;

    return 0;

####################################################################
# function to run a test
def runTest():
    # get number of classes
    N = int(raw_input());

    # the classList will be a list of lists. Each sublist will contain
    # the classes that this one inherits from
    count = 0;
    classList = [];

    arrayIndex = 1;
    totalNodes = N;
    while (N > 0):
        i = raw_input();
        M = i.split(" ");
        Mi = (int)(M[0]);
        M.pop(0);               # remove first element

        classList.append(M);    # add remaining elements to list
        N = N - 1;
        arrayIndex = arrayIndex + 1;

    # now the array elements are in, it's time to pick a start node and
    # traverse from there. If the node has already been visited, there's
    # no need to start from this node
    visited = [0]*totalNodes;
    for i in range (1, totalNodes):
        index = i - 1;
        if (visited[index] == 0):
            result = traverse(classList, visited, totalNodes, i);
            if (result == 1):
                return "Yes";

    return "No";

####################################################################

def main():
    # get the number of tests
    T = int(raw_input());
    caseNum = 1;

    # start the clock
    startTime = int(round(time.time() * 1000))

    while (T > 0):
        result = runTest();
        string = "Case #{0:d}: {1:s}".format(caseNum, result);
        print string;
        T = T - 1;
        caseNum = caseNum + 1;

    endTime = int(round(time.time() * 1000))
    timeStr = "That took {0:d} ms\n".format(endTime - startTime);
#    print timeStr;

####################################################################

main();
