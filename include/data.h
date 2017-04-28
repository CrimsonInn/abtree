#ifndef DABTREE_DATA_H_
#define DABTREE_DATA_H_

#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <string>
#include "matrix.h"
#include "data.pb.h"
#include "matrix.pb.h"

class DataProvider {
public:

  DataProvider() { row_index = 0; }
  DataProvider(const std::string &file_name);

  size_t num_samples() {
    return sample_ptr_->GetHeight();
  }

  size_t num_feas() {
    return sample_ptr_->GetWidth();
  }

  FeaType get_fea_type(size_t index) {
    return sample_ptr_->fea_type(index);
  }

  std::vector<FeaType> get_fea_types() {
    return sample_ptr_->fea_types();
  }

  Value get_value(size_t row_id, size_t col_id) {
    return (*sample_ptr_)(row_id, col_id);
  }

  void set_fea_type(const std::vector<FeaType>& types) {
    sample_ptr_->SetType(types);
  }

  void add_data(const std::vector<Value>& value) {
    sample_ptr_->Add(value);
  }

  MatrixPtr get_samples() {
    MatrixPtr samptr = sample_ptr_;
    return samptr;
  }

  void get_next_batch(MatrixPtr batch_ptr, size_t batch_size);

  void print_samples(size_t row_num);

private:
  int row_index;
  MatrixPtr sample_ptr_ = std::make_shared<Matrix>();
};


#endif  // DABTREE_DATA_H_
