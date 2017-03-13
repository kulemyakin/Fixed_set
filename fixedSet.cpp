#include <vector>
#include <iostream>
#include <algorithm>
#include <random>

const int64_t kPrime = 10000000019LL;  // Prime Number greater than the largest
                                       // int. This prime will give us no
                                       // collisions, because 10,000,000,019 >
                                       // 2^32, so we can`t have two integers
                                       // that differ on the chosen prime
const int kSizeBound = 5;

class OptionalInt {
 public:
  OptionalInt()
    : value_(0), isInitialized_(false)
  {}

  explicit OptionalInt(int value)
    : value_(value), isInitialized_(true)
  {}

  void Initialize(const int value) {
    value_ = value;
    isInitialized_ = true;
  }

  int value() const {
    return value_; 
  }

  bool isInitialized() const {
    return isInitialized_; 
  }

 private:
  int value_;
  bool isInitialized_;
};

class HashFunction {
 public:
  HashFunction(){};
  HashFunction(const int64_t slope, const int64_t intercept)
      : slope_(slope), intercept_(intercept), functionPrime_(kPrime) 
  {}

  HashFunction(const int64_t slope, const int64_t intercept,
               const int64_t prime)
      : slope_(slope), intercept_(intercept), functionPrime_(kPrime)
  {}

  int getValue(const int key, const int size) const {
    int64_t result = (slope_ * key + intercept_) % functionPrime_;
    result = result % size;
    if (result < 0) {
      result += size;
    }
    return result;
  }

 private:
  int64_t slope_;
  int64_t intercept_;
  int64_t functionPrime_;
};

HashFunction GetRandomHashFunction(std::mt19937& randomGenerator) {
  std::uniform_int_distribution<int64_t> slopeDistribution(1, kPrime);
  std::uniform_int_distribution<int64_t> interceptDistribution(0, kPrime);
  return HashFunction(slopeDistribution(randomGenerator),
                      interceptDistribution(randomGenerator), kPrime);
}

std::vector<size_t> FindKeyDistribution(const HashFunction& hash,
                                        const std::vector<int>& numbers) {
  size_t range = numbers.size();
  std::vector<size_t> distribution(numbers.size(), 0);
  for (const auto& intToSet : numbers) {
    ++distribution[hash.getValue(intToSet, range)];
  }
  return distribution;
}

int64_t CalculateSumOfSquares(const std::vector<size_t>& keyDistribution) {
  int64_t currentSum = 0;
  for (const auto& bucketSize : keyDistribution) {
    currentSum += bucketSize * bucketSize;
  }
  return currentSum;
}

class Bucket {
 public:
  void Initialize(const std::vector<int>& intsToSet,
                  std::mt19937& randomGenerator) {
    size_t bucketSize = intsToSet.size() * intsToSet.size();
    std::vector<size_t> distribution;
    do {
      OptionalInt null;
      boxes_.assign(bucketSize, null);
      bucketHash_ = GetRandomHashFunction(randomGenerator);
      distribution = FindKeyDistribution(bucketHash_, intsToSet);
    } while (
        CalculateSumOfSquares(distribution) !=
        intsToSet.size());  // that condition means that we have no collisions
    for (const auto& curInt : intsToSet) {
      size_t hashIndex = bucketHash_.getValue(curInt, boxes_.size());
      boxes_[hashIndex].Initialize(curInt);
    }
  }

  bool Contains(const int intToSearch) const {
    if (boxes_.empty()) {
      return false;
    }
    size_t hashIndex = bucketHash_.getValue(intToSearch, boxes_.size());
    return (boxes_[hashIndex].isInitialized() &&
            boxes_[hashIndex].value() == intToSearch);
  }

 private:
  std::vector<OptionalInt> boxes_;
  HashFunction bucketHash_;
  int64_t bucketPrime_;
};

class FixedSet {
 public:
  void Initialize(const std::vector<int>& intsToSet) {
    std::mt19937 randomGenerator(std::random_device {}());
    size_ = intsToSet.size();
    buckets_.resize(size_);
    int64_t memoryToSet = kSizeBound * size_ + 1;
    while (memoryToSet > kSizeBound * size_) {
      tableHash_ = GetRandomHashFunction(randomGenerator);
      std::vector<size_t> currentDistribution =
          FindKeyDistribution(tableHash_, intsToSet);
      memoryToSet = CalculateSumOfSquares(currentDistribution);
    }
    std::vector<std::vector<int>> intsToSetInBucket;
    intsToSetInBucket.assign(size_, {});
    for (const auto& curInt : intsToSet) {
      size_t hashIndex = tableHash_.getValue(curInt, size_);
      intsToSetInBucket[hashIndex].push_back(curInt);
    }
    for (size_t bucketNumber = 0; bucketNumber < size_; ++bucketNumber) {
      buckets_[bucketNumber].Initialize(intsToSetInBucket[bucketNumber],
                                        randomGenerator);
    }
  }

  bool Contains(const int intToSearch) const {
    size_t hashIndex = tableHash_.getValue(intToSearch, size_);
    return buckets_[hashIndex].Contains(intToSearch);
  }

 private:
  std::vector<Bucket> buckets_;
  int size_;
  HashFunction tableHash_;
};

void ReadInts(std::vector<int>* intsToSet,
              std::istream& inputStream = std::cin) {
  size_t size;
  inputStream >> size;
  for (size_t index = 0; index < size; ++index) {
    int inputedInt;
    inputStream >> inputedInt;
    intsToSet->push_back(inputedInt);
  }
}

std::vector<bool> findInts(const std::vector<int>& intsToSet,
                           const std::vector<int>& intsToSearch) {
  FixedSet newSet;
  newSet.Initialize(intsToSet);
  std::vector<bool> answer;
  for (const auto& number : intsToSearch) {
    answer.push_back(newSet.Contains(number));
  }
  return answer;
}

void writeAnswer(const std::vector<bool>& answer) {
  for (const auto& found : answer) {
    if (found) {
      std::cout << "Yes\n";
    } else {
      std::cout << "No\n";
    }
  }
}

int main() {
  std::ios_base::sync_with_stdio(false);
  std::vector<int> intsToSet;
  std::vector<int> intsToSearch;
  ReadInts(&intsToSet);
  ReadInts(&intsToSearch);
  std::vector<bool> answer = findInts(intsToSet, intsToSearch);
  writeAnswer(answer);
}
