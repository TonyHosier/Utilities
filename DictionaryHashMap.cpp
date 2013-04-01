#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>
#include <vector>
#include <map>

#include "Stringhash.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Test to compare a standard associative array (map) lookup vs a hashmap lookup
//
// In both cases, each key would normally be a string, but it's hashed to create a
// 32-bit int. It's obviously quicker to do an integer compare rather than a string
// compare.
//
// In the standard method, all words are added to 1 monolithic map. Searching this will
// be logN, but for large N, there could be a performance hit.
//
// A hashmap is an array of maps. The idea is to break this monolithic map into an array
// of smaller maps, which can be searched much quicker. The two problems are how large to
// make this array and what to use as an index into it.
//
// The key is to make the number of array elements, or buckets, small enough so that all
// buckets have at least one entry (no wasted space) and large enough so that the maps in
// each bucket are as small as possible.
// Ideally, each bucket would contain a map with exactly 1 element, but the actual number
// will be determined by the data set and the hashing algorithm chosen.
// The number of buckets should be a prime number, since prime numbers seem to work best
// For large N, search time should be linear. The array size will increase to accommodate
// more data but the maps inside each array element will still be the same size. It won't
// be O(1), but it will be substantially better than logN.
//
// The index is simply created by deviding the key by the size of the hashmap and taking
// the modulo of the result
//
// To give both methods a head start, the dictionary can first be broken into 26 arrays,
// one for each start letter of the word. This will reduce collisions between hashes.
// Just for comparison, a single monolithic associative array is added for good measure
//
// The test case here seems to favor a bucket size around 1/8 the size of the data set.
// The nearest prime number to this value is chosen.
// The main speedup comes from splitting the large data set into 26 smaller arrays, so
// if data can be split up in a trivial way like this to begin with, then all the better
// (ie a list of people's names could be stored as 26 tables listed by last name).
// There is a 10% speedup by using the hash map so since it doesn't take any additional
// space seems worth doing.
//
// For static data, as in this example, the array size can be optimized but for constantly
// changing data, the array and maps may need to be rebuilt if the maps become unbalanced.
// This could be an expensive operation and will need to be done with caution.

using namespace std;

static const int    NUM_ITERATIONS = 1000000;   // number of test iterations to perform
static const int    NUM_LETTERS = 26;           // number of letters in the alphabet
static const int    NUM_TEST_CLASSES = 3;

static int GetNearestPrimeNumberTo(int number)
{
    // Ensure this table is big enough if the dataset changes
    static const int primes[] =
    {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97,
        101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197,
        199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 
        317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439,
        443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571,
        577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691,
        701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829,
        839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977,
        983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091,
        1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217,
        1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321,
        1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471,
        1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583,
        1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709,
        1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861,
        1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993,
        1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069, 2081, 2083, 2087, 2089, 2099, 2111,
        2113, 2129, 2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243, 2251,
        2267, 2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357, 2371, 2377,
        2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521,
        2531, 2539, 2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617, 2621, 2633, 2647, 2657, 2659, 2663,
        2671, 2677, 2683, 2687, 2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749, 2753, 2767,
        2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861, 2879, 2887, 2897, 2903,
        2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999, 3001, 3011, 3019, 3023, 3037, 3041, 3049,
        3061, 3067, 3079, 3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187, 3191, 3203, 3209,
        3217, 3221, 3229, 3251, 3253, 3257, 3259, 3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331, 3343,
        3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413, 3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491,
        3499, 3511, 3517, 3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607, 3613,
        3617, 3623, 3631, 3637, 3643, 3659, 3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727, 3733, 3739,
        3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823, 3833, 3847, 3851, 3853, 3863, 3877, 3881, 3889,
        3907, 3911, 3917, 3919, 3923, 3929, 3931, 3943, 3947, 3967, 3989, 4001, 4003, 4007, 4013, 4019, 4021,
        4027, 4049, 4051, 4057, 4073, 4079, 4091, 4093
    };

    // find "number" using the usual binary search. If an exact index can't be found, take the lowest
    int firstIndex = 0;
    int lastIndex = sizeof(primes) / sizeof(primes[0]);

    do
    {
        int mid = (firstIndex + lastIndex) / 2;
        if (number > primes[mid])
        {
            firstIndex = mid;
        }
        else
        {
            lastIndex = mid;
        }
    }
    while ((lastIndex - firstIndex) > 1);

    return primes[firstIndex];
}

struct KVPair
{
    unsigned int _key;
    string       _value;
};

// Class describing the dictionary. It consists of an array of key-value pairs, where
// the word (value) has been pre-hashed (key)
// All words are stored here. The hash map classes have access to this data with which
// to build their own data structures
class   Dictionary
{
public:
    Dictionary()
    {
        for (int loop = 0; loop < NUM_LETTERS; loop++)
        {
            _numWords[loop] = 0;
        }
    }
    
    ~Dictionary()
    {
    }

    // Read the dictionary, hash the key string and store in the K-V pair array
    bool    ReadFile(char* fileName)
    {
        string line;
        ifstream myfile(fileName);

        if (myfile.is_open())
        {
            while ( myfile.good() )
            {
                getline( myfile, line );
                const char* str = line.c_str();
                if (strlen(str) > 0)
                {
                    StringHash hash = StringHash( str );

                    KVPair pair;
                    pair._key = hash;
                    pair._value = line;
                    _stringArray.push_back(pair);
                    
                    int wordIndex = str[0] - 'a';
                    _numWords[wordIndex]++;
                }
            }
        }
        else
        {
            return false;
        }
        myfile.close();
        return true;
    }

    const int GetWordCount(int index) const
    {
        return _numWords[index];
    }

    const int GetSize() const
    {
        return _stringArray.size();
    }

    const KVPair&   GetKVPair(int index) const
    {
        return _stringArray[index];
    }

    const string&   GetString(int index) const
    {
        return _stringArray[index]._value;
    }

private:

    // the KV pairs of words read in
    vector<KVPair>  _stringArray;
    int             _numWords[NUM_LETTERS];
};

// base class for hash map. Will handle all common functionality between the different
// Hash map derived classes
// It also contains a collision list of words that didn't make it into the hash map
class   HashMapBase
{
public:
    HashMapBase()
    {
    }

    virtual ~HashMapBase()
    {
    }

    virtual void CreateMap(Dictionary* ) = 0;
    virtual bool Find(const string& ) const = 0;

    void    RunTest(Dictionary* dictionary)
    {
        int foundCount = 0;
        int size = dictionary->GetSize();

        int startTime = timeGetTime();
        for (int loop = 0; loop < NUM_ITERATIONS; loop++)
        {
            int index = loop % size;
            const string& word = dictionary->GetString(index);
            if (Find(word))
            {
                foundCount++;
            }
        }
        int endTime = timeGetTime() - startTime;
        cout << endTime << "ms" << " to test map (found " << foundCount << "/" << NUM_ITERATIONS << ")" << endl;
    }

    // Find "word" in the collision table
    // return true if found, false otherwise.
    bool    FindCollision(const StringHash& key, const string& word) const
    {
        map<StringHash, vector<string> >::const_iterator it;     // iterator for collisions
        it = _collisions.find(key);
        if (it != _collisions.end())
        {
            const vector<string>& wordList = it->second;
            for (vector<string>::const_iterator iter = wordList.begin(); iter != wordList.end(); ++iter)
            {
                if ( strcmp( word.c_str(), (*iter).c_str()) == 0)
                    return true;
            }
        }
        return false;
    }

protected:
    void    ResolveCollisions(map<StringHash, string>& wordMap)
    {
        // resolve any collisions. Move collided objects still in the associative array
        // into the collision table
        map<StringHash, vector<string> >::iterator colIt;     // iterator for collisions
        for (colIt = _collisions.begin(); colIt != _collisions.end(); ++colIt)
        {
            StringHash  hash = (*colIt).first;
            map<StringHash, string>::iterator it = wordMap.find(hash);
            if (it != wordMap.end() )
            {
                (*colIt).second.push_back( (*it).second);
                cout << "key " << (*it).first << " moving to collisions: value " << (*it).second << endl;
                wordMap.erase(hash);
            }
            else
            {
                cout << "ERROR: Collision not in original map. This should NEVER happen" << endl;
            }
        }
    }

    void    AddCollision(const KVPair& kvPair, const StringHash& hash)
    {
        // This key has collided with something already in the map, so add this to the
        // collisions table
        map<StringHash, vector<string> >::iterator colIt = _collisions.find(hash);
        if (colIt != _collisions.end())
        {
            // already in the list, add it here
            (*colIt).second.push_back( kvPair._value);
            cout << "key " << kvPair._key << " already in list: value " << kvPair._value << endl;
        }
        else
        {
            // add a new set of data
            vector<string>  collision;
            collision.push_back( kvPair._value);
            _collisions.insert(make_pair(hash, collision));
            cout << "key " << kvPair._key << " collided with value " << kvPair._value << endl;
        }
    }

    map<StringHash, vector<string> >    _collisions;
};

// A class describing a large monolithic map of words
// This would be the standard implementation in any
// dictionary-based program.
class MonolithicMap : public HashMapBase
{
public:
    MonolithicMap()
    {
    }

    virtual ~MonolithicMap()
    {
    }

    void    CreateMap(Dictionary* dictionary)
    {
        int size = dictionary->GetSize();
        for (int loop = 0; loop < size; loop++)
        {
            const KVPair& kvPair = dictionary->GetKVPair(loop);

            // create the hash for the current string
            StringHash hash = StringHash( kvPair._key );

            // insert it into the standard associative array if it isn't there already
            if (_wordMap.find(hash) == _wordMap.end())
            {
                _wordMap.insert(make_pair(hash, kvPair._value));
            }
            else
            {
                AddCollision(kvPair, hash);
            }
        }
        ResolveCollisions(_wordMap);
    }

    bool    Find(const string& wordToFind) const
    {
        StringHash key = StringHash( wordToFind );
        if (_wordMap.find(key) != _wordMap.end())
            return true;
        if (FindCollision(key, wordToFind) )
            return true;
        return false;
    }

private:
    // store for the standard map of HashKey<->string
    map<StringHash, string>             _wordMap;
};

// A class describing large monolithic maps; one for each individual letter.
// This will use the first letter as a lookup. It should also reduce the likelyhood
// of collisions
class MonolithicLetterMap : public HashMapBase
{
public:
    MonolithicLetterMap()
    {
    }

    virtual ~MonolithicLetterMap()
    {
    }

    void    CreateMap(Dictionary* dictionary)
    {
        int size = dictionary->GetSize();
        for (int loop = 0; loop < size; loop++)
        {
            const KVPair& kvPair = dictionary->GetKVPair(loop);
            const char* str = kvPair._value.c_str();
            int index = str[0] - 'a';

            // create the hash for the current string
            StringHash hash = StringHash( kvPair._key );

            // insert it into the standard associative array if it isn't there already
            if (_wordMap[index].find(hash) == _wordMap[index].end())
            {
                _wordMap[index].insert(make_pair(hash, kvPair._value));
            }
            else
            {
                AddCollision(kvPair, hash);
            }
        }
        for (int loop = 0; loop < NUM_LETTERS; loop++)
        {
            ResolveCollisions(_wordMap[loop]);
        }
    }

    bool    Find(const string& wordToFind) const
    {
        int letterIndex = wordToFind[0] - 'a';
        StringHash key = StringHash( wordToFind );
        if (_wordMap[letterIndex].find(key) != _wordMap[letterIndex].end())
            return true;
        if (FindCollision(key, wordToFind) )
            return true;
        return false;
    }

private:
    // store for the standard map of HashKey<->string
    map<StringHash, string>             _wordMap[NUM_LETTERS];
};

// The hashMap structure for each list of words beginning with a certain letter
// since all letters will have a different amount of words in, the _arraySize
// will be used to optimally pick the best array size, ie the array will be bigger
// for words starting with "e" than for words starting with "x"
struct HashArray
{
    int                                _arraySize;
    vector< map<StringHash, string> >  _hashMap;
};

class HashMap : public HashMapBase
{
public:
    HashMap()
    {
    }

    ~HashMap()
    {
    }

    void    CreateMap(Dictionary* dictionary)
    {
        for (int loop = 0; loop < NUM_LETTERS; loop++)
        {
            HashArray&  hashMap = _hashMap[loop];
            hashMap._hashMap.clear();

            int arraySize = dictionary->GetWordCount(loop) / 8;
            arraySize = GetNearestPrimeNumberTo(arraySize);
            hashMap._arraySize = arraySize;
            hashMap._hashMap.resize(hashMap._arraySize);
        }

        // move words from the dictionary into the map
        int size = dictionary->GetSize();
        for (int loop = 0; loop < size; loop++)
        {
            const KVPair& kvPair = dictionary->GetKVPair(loop);
            const char* str = kvPair._value.c_str();
            int letterIndex = str[0] - 'a';

            HashArray&  hashMap = _hashMap[letterIndex];

            // take modulo and insert it into the hash map.
            unsigned int bucketIndex = kvPair._key % hashMap._arraySize;

            // create the hash for the current string
            StringHash hash = StringHash( kvPair._key );

            // insert it into the standard associative array if it isn't there already
            if (hashMap._hashMap[bucketIndex].find(hash) == hashMap._hashMap[bucketIndex].end())
            {
                hashMap._hashMap[bucketIndex].insert(make_pair(hash, kvPair._value));
            }
            else
            {
                AddCollision(kvPair, hash);
            }
        }

        for (int letterLoop = 0; letterLoop < NUM_LETTERS; letterLoop++)
        {
            HashArray&  hashMap = _hashMap[letterLoop];
            int arraySize = hashMap._arraySize;
            for (int loop = 0; loop < arraySize; loop++)
            {
                ResolveCollisions(hashMap._hashMap[loop]);
            }
        }

        // dump out stats for buckets
        for (int letterLoop = 0; letterLoop < NUM_LETTERS; letterLoop++)
        {
            int numEmptyBuckets = 0;
            int maxBucketSize = 0;
            HashArray&  hashMap = _hashMap[letterLoop];
            int numBuckets = hashMap._arraySize;
            for (int loop = 0; loop < numBuckets; loop++)
            {
                map<StringHash, string>&    bucketMap = hashMap._hashMap[loop];
                int size = bucketMap.size();
                if (size == 0)
                {
                    numEmptyBuckets++;
                }
                if (size > maxBucketSize)
                {
                    maxBucketSize = size;
                }
            }
            cout << "Bucket Size for " << letterLoop << ": " << numBuckets << ", Num Empty buckets: " << numEmptyBuckets << ", maxBucketSize: " << maxBucketSize << endl;
        }
    }

    bool    Find(const string& wordToFind) const
    {
        int letterIndex = wordToFind[0] - 'a';
        StringHash key = StringHash( wordToFind );
        const HashArray&  hashMap = _hashMap[letterIndex];
        unsigned int bucketIndex = key % hashMap._arraySize;
        if (hashMap._hashMap[bucketIndex].find(key) != hashMap._hashMap[bucketIndex].end())
            return true;
        if (FindCollision(key, wordToFind) )
            return true;
        return false;
    }

private:
    HashArray       _hashMap[NUM_LETTERS];
};

/////////////////////////////////////////////////////////////////////////////////////////
// The main test program here
int main(int argc, char**argv)
{
    Dictionary* dictionary = new Dictionary();
    HashMapBase* testMap[NUM_TEST_CLASSES];

    testMap[0] = new MonolithicMap();
    testMap[1] = new MonolithicLetterMap();
    testMap[2] = new HashMap;

    cout << "Reading Dictionary" << endl;
    if (dictionary->ReadFile("wordlist.txt"))
    {
        for (int loop = 0; loop < NUM_TEST_CLASSES; loop++)
        {
            cout << "Creating Map " << loop << endl;
            testMap[loop]->CreateMap(dictionary);
            cout << "Running test " << loop << endl;
            testMap[loop]->RunTest(dictionary);
            cout << "Deleting " << loop << endl;
            delete testMap[loop];
        }
    }
    delete dictionary;
    return 0;
}
