import time
import math

MAX_INDEX = 46341

# This is from the Facebook hacker cup 2011 qualification round

# A double-square number is an integer X which can be expressed as the sum of
# two perfect squares. For example, 10 is a double-square because 10 = 3^2 + 1^2.
# Your task in this problem is, given X, determine the number of ways in which
# it can be written as the sum of two squares. For example, 10 can only be
# written as 3^2 + 1^2 (we don't count 1^2 + 3^2 as being different). On the
# other hand, 25 can be written as 5^2 + 0^2 or as 4^2 + 3^2.
#
# Input
# You should first read an integer N, the number of test cases. The next N
# lines will contain N values of X.
# Constraints
# 0 = X = 2147483647
# 1 = N = 100
# Output
# For each value of X, you should output the number of ways to write X as the
# sum of two squares.

# 5
# 10
# 25
# 3
# 0
# 1

# Case #1: 1
# Case #2: 2
# Case #3: 0
# Case #4: 1
# Case #5: 1

# function to run a test
def runTest(squareTable):
    # get the value if X specified in the problem
    X = int(raw_input());

    sum = 0;        # assign the sum variable
    # go through the list and try each number in the square table. Subtract
    # this value from X. If the result is 0, it's a square number so 0^2 can
    # also be used. Therefore it satifies the condition so increment the sum
    # counter.
    # if the result is non-zero, take the square root of the remainder and
    # use that as an array index to try. Due to square root inaccuracies,
    # the result is rounded down and this index and the next index are both
    # tried. This targets the number range of the second value so that the
    # brute force method is avoided

    index = 0;
    squaresTable = [];
    while (index < MAX_INDEX):
        value = squareTable[index];
        remainder = X - value;

        # if the remainder is less than zero, then no other values
        # need to be considered
        if (remainder < 0):
            return sum;

        else:
            root = math.sqrt( remainder );
            index2 = (int)(math.floor(root));

            # eliminate duplicates, though it's possible to have both values
            if (index2 >= index):
                if (value + squareTable[index2] == X):
                    sum = sum + 1;
                else:
                    index2 = index2 + 1;
                    if (index2 < MAX_INDEX):
                        if (value + squareTable[index2] == X):
                            sum = sum + 1;

        index = index + 1;

    return sum;

def main():
    # get the number of tests
    N = int(raw_input());
    caseNum = 1;

    # start the clock
    startTime = int(round(time.time() * 1000))

    # build the table of square numbers. Only needs to be done once
    # the square root of 2147483647 is 46340.9, so use 46340. This
    # array size is reasonable. This is defined in MAX_INDEX at the top
    # of this file

    index = 0;
    squaresTable = [];
    while (index < MAX_INDEX):
        squaresTable.append(index * index);
        index = index + 1;

    while (N > 0):
        numSquares = runTest(squaresTable);
        string = "Case #{0:d}: {1:d}".format(caseNum, numSquares);
        print string;
        N = N - 1;
        caseNum = caseNum + 1;

    endTime = int(round(time.time() * 1000))
    timeStr = "\nThat took {0:d} ms".format(endTime - startTime);
    print timeStr;

####################################################################

main();

