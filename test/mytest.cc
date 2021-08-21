#include <cassert>
#include <string>

#include "leveldb/comparator.h"
#include "leveldb/db.h"
#include "leveldb/write_batch.h"

#include "gtest/gtest.h"

class TwoPartComparator : public leveldb::Comparator {
 public:
  // Three-way comparison function:
  //   if a < b: negative result
  //   if a > b: positive result
  //   else: zero result
  int Compare(const leveldb::Slice& a, const leveldb::Slice& b) const {
    int a1, a2, b1, b2;
    // ParseKey(a, &a1, &a2);
    // ParseKey(b, &b1, &b2);
    if (a1 < b1) return -1;
    if (a1 > b1) return +1;
    if (a2 < b2) return -1;
    if (a2 > b2) return +1;
    return 0;
  }

  // Ignore the following methods for now:
  const char* Name() const { return "TwoPartComparator"; }
  void FindShortestSeparator(std::string*, const leveldb::Slice&) const {}
  void FindShortSuccessor(std::string*) const {}
};

TEST(DBTest, open) {
  leveldb::DB* db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
  assert(status.ok());
  std::string key("mark");
  std::string value("marktest");
  auto s = db->Put(leveldb::WriteOptions(), key, value);
  std::string value2;
  db->Get(leveldb::ReadOptions(), key, &value2);
  std::cout << "get value2 " << value2 << std::endl;

  std::string value1;
  s = db->Get(leveldb::ReadOptions(), key, &value1);
  if (s.ok()) {
    std::cout << "batch test" << std::endl;
    leveldb::WriteBatch batch;
    batch.Delete(key);
    batch.Put(key, value1);
    s = db->Write(leveldb::WriteOptions(), &batch);
  }

  leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    std::cout << it->key().ToString() << ": " << it->value().ToString()
              << std::endl;
  }
  assert(it->status().ok());  // Check for any errors found during the scan

  leveldb::ReadOptions option;
  option.snapshot = db->GetSnapshot();
  // ... apply some updates to db...
  db->Put(leveldb::WriteOptions(), key, value + "snapshot");
  leveldb::Iterator* iter = db->NewIterator(option);
  std::cout << "after update: " << std::endl;
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    std::cout << it->key().ToString() << ": " << it->value().ToString()
              << std::endl;
  }
  // ... read using iter to view the state when the snapshot was created...
  delete iter;
  db->ReleaseSnapshot(option.snapshot);
  delete it;
  delete db;
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}