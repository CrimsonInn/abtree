#include <iostream>
#include <fstream>
#include <string>
#include "proto_func.h"
#include <glog/logging.h>

// const size_t MAX_NODE_SIZE = 64;

// bool write_batch_data(BatchPtr batch_data, const std::string& file_name) {

//   LOG(INFO) << "Begin write batch data ...";

//   unsigned int fea_type_nums = batch_data->fea_types.size();
//   unsigned int value_rows = batch_data->samples.size();
//   unsigned int value_cols = 0;
//   if(value_rows > 0){
//   	value_cols = batch_data->samples[0].size();
//   }

//   // TODO: do we need to check fea_type_nums ?= value_rows?

//   // Verify that the version of the protobuf that we linked against is
//   // compatible with the version of the headers we compiled against.
//   GOOGLE_PROTOBUF_VERIFY_VERSION;

//   rawdata::Batch write_batch_data;

//   // Read the existing address book.
//   std::fstream input(file_name, std::ios::in | std::ios::binary);
//   if (!input) {
//     LOG(INFO) << file_name << ": File not found. Creating a new file.";
//   } else if (!write_batch_data.ParseFromIstream(&input)) {
//     LOG(ERROR) << "Failed to parse batch data.";
//     return false;
//   }

//   // write bach data into protobuf
//   write_batch_data.set_rows(value_rows);
//   write_batch_data.set_cols(value_cols);

//   for (auto fea_type : batch_data->fea_types) {
//   	write_batch_data.add_fea_types(rawdata::FeaType(fea_type));
//   }

//   for (auto sample_row : batch_data->samples) {
//   	for (size_t i = 0; i < sample_row.size(); ++i) {
//   		if (batch_data->fea_types[i] == CONT){
//   			write_batch_data.add_samples()->set_v(sample_row[i].v);
//   		} else if (batch_data->fea_types[i] == DISC) {
//   			write_batch_data.add_samples()->set_cls(sample_row[i].cls);
//   		} else if (batch_data->fea_types[i] == RANK) {
//   			write_batch_data.add_samples()->set_level(sample_row[i].level);
//   		} else {
//   			LOG(ERROR) << "Batch data type error.";
//   			return false;
//   		}
//   	}
//   }

// 	// Write the new address book back to disk.
//   std::fstream output(file_name, std::ios::out | std::ios::trunc | std::ios::binary);
//   if (!write_batch_data.SerializeToOstream(&output)) {
//     LOG(ERROR) << "Failed to write batch data.";
//     return false;
//   } else {
//     LOG(INFO) << "Success!";
//   }

//   // Optional:  Delete all global objects allocated by libprotobuf.
//   google::protobuf::ShutdownProtobufLibrary();

//   return true;

// }

/*
bool write_matrix_data(MatrixPtr matrixptr, rawdata::Matrix* write_matrix) {

  LOG(INFO) << "Begin write rawdata matrix ...";

  // TODO: 

  // write tree data into protobuf

  unsigned int width = matrixptr->GetWidth();
  unsigned int height = matrixptr->GetHeight();
  write_matrix->set_width(width);
  write_matrix->set_height(height);

  for (size_t i = 0; i < width; ++i) {
    write_matrix->add_fea_types(rawdata::FeaType(matrixptr->fea_type(i)));
  }

  for (size_t i = 0; i < height; ++i) {
    for (size_t j = 0; j < width; ++j) {
      auto &matrix = (*matrixptr);
      std::cout << matrixptr->fea_type(j) << std::endl;
      if (matrixptr->fea_type(j) == CONT) { 
        write_matrix->add_data()->set_v(matrix(i, j).v);
      } else if (matrixptr->fea_type(j) == DISC) {
        write_matrix->add_data()->set_cls(matrix(i, j).cls);
      } else if (matrixptr->fea_type(j) == RANK) {
        write_matrix->add_data()->set_level(matrix(i, j).level);
      } else {
        LOG(ERROR) << "Batch data type error.";
        return false;
      }
    }
  }

  return true;

}


bool write_batch_data(BatchPtr batch_data_ptr, const std::string& file_name) {

  LOG(INFO) << "Begin write batch data ...";

  // TODO: do we need to check fea_type_nums ?= value_rows?

  // Verify that the version of the protobuf that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  //rawdata::Batch write_batch_data;
  rawdata::Matrix write_batch_data;

  // Read the existing address book.
  std::fstream input(file_name, std::ios::in | std::ios::binary);
  if (!input) {
    LOG(INFO) << file_name << ": File not found. Creating a new file.";
  } else if (!write_batch_data.ParseFromIstream(&input)) {
    LOG(ERROR) << "Failed to parse batch data.";
    return false;
  }

  // write bach data into protobuf
  MatrixPtr sample_ptr = batch_data_ptr->get_samples();
  // if (write_matrix_data(sample_ptr, write_batch_data.set_samples())){
  //   LOG(INFO) << "Finish write batch data." ;
  // }

  // rawdata::Matrix &write_matrix = write_batch_data.set_samples();
  rawdata::Matrix &write_matrix = write_batch_data;

  unsigned int width = sample_ptr->GetWidth();
  unsigned int height = sample_ptr->GetHeight();
  write_matrix.set_width(width);
  write_matrix.set_height(height);

  for (size_t i = 0; i < width; ++i) {
    write_matrix.add_fea_types(rawdata::FeaType(sample_ptr->fea_type(i)));
  }

  for (size_t i = 0; i < height; ++i) {
    for (size_t j = 0; j < width; ++j) {
      auto &matrix = (*sample_ptr);
      // std::cout << sample_ptr->fea_type(j) << std::endl;
      if (sample_ptr->fea_type(j) == CONT) { 
        write_matrix.add_data()->set_v(matrix(i, j).v);
      } else if (sample_ptr->fea_type(j) == DISC) {
        write_matrix.add_data()->set_cls(matrix(i, j).cls);
      } else if (sample_ptr->fea_type(j) == RANK) {
        write_matrix.add_data()->set_level(matrix(i, j).level);
      } else {
        LOG(ERROR) << "Batch data type error.";
        return false;
      }
    }
  }

  // Write the new address book back to disk.
  std::fstream output(file_name, std::ios::out | std::ios::trunc | std::ios::binary);
  if (!write_batch_data.SerializeToOstream(&output)) {
    LOG(ERROR) << "Failed to write batch data.";
    return false;
  } else {
    LOG(INFO) << "Success!";
  }

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();

  return true;

}
*/

// bool write_matrix_data(MatrixPtr matrixptr, dabtree::Matrix write_matrix) {

//   LOG(INFO) << "Begin write matrix ...";

//   // TODO: 

//   // write tree data into protobuf

//   unsigned int width = matrixptr->GetWidth();
//   unsigned int height = matrixptr->GetHeight();
//   write_matrix.set_width(width);
//   write_matrix.set_height(height);

//   for (size_t i = 0; i < width; ++i) {
//     write_matrix.add_fea_types(dabtree::FeaType(matrixptr->fea_type(i)));
//   }

//   for (size_t i = 0; i < height; ++i) {
//     for (size_t j = 0; j < width; ++j) {
//       auto &matrix = (*matrixptr);
//       std::cout << matrixptr->fea_type(j) << std::endl;
//       if (matrixptr->fea_type(j) == CONT) { 
//         write_matrix.add_data()->set_v(matrix(i, j).v);
//       } else if (matrixptr->fea_type(j) == DISC) {
//         write_matrix.add_data()->set_cls(matrix(i, j).cls);
//       } else if (matrixptr->fea_type(j) == RANK) {
//         write_matrix.add_data()->set_level(matrix(i, j).level);
//       } else {
//         LOG(ERROR) << "Batch data type error.";
//         return false;
//       }
//     }
//   }

//   return true;

// }


// bool write_tree(RegTreePtr treeptr, const std::string& file_name) {

// 	LOG(INFO) << "Begin write tree ...";

//   // TODO: 

//   // Verify that the version of the protobuf that we linked against is
//   // compatible with the version of the headers we compiled against.
//   GOOGLE_PROTOBUF_VERIFY_VERSION;

//   dabtree::RegTree write_tree;

//   // Read the existing address book.
//   std::fstream input(file_name, std::ios::in | std::ios::binary);
//   if (!input) {
//     LOG(INFO) << file_name << ": File not found. Creating a new file.";
//   } else if (!write_tree.ParseFromIstream(&input)) {
//     LOG(ERROR) << "Failed to parse tree data.";
//     return false;
//   }

//   // write tree data into protobuf
//   // unsigned int tree_id = tree->id;
//   // float tree_weight = tree->weight;
//   // write_tree.set_id(tree_id);
//   // write_tree.set_weight(tree_weight);

// //bool write_matrix_data(MatrixPtr matrixptr, dabtree::Matrix write_matrix) {

//   MatrixPtr split_fea_ptr = treeptr->GetSplitFea();
//   if (write_matrix_data(split_fea_ptr, write_tree.split_fea())) {
//     LOG(INFO) << "Finish write fea_types." ;
//   }

//   MatrixPtr split_value_ptr = treeptr->GetSplitValue();
//   if (write_matrix_data(split_value_ptr, write_tree.split_value())) {
//     LOG(INFO) << "Finish write fea_value." ;
//   }


//  //  for (auto fea_type : tree->fea_types) {
//  //  	write_tree.add_fea_types(dabtree::FeaType(fea_type));
//  //  }

//  //  for (auto sf : tree->split_fea) {
//  //  	write_tree.add_split_fea(sf);
//  //  }

// 	// for (size_t i = 0; i < tree->fea_types.size(); ++i) {
// 	// 	if (tree->fea_types[i] == CONT){
// 	// 		write_tree.add_split_value()->set_v(tree->split_value[i].v);
// 	// 	} else if (tree->fea_types[i] == DISC) {
// 	// 		write_tree.add_split_value()->set_cls(tree->split_value[i].cls);
// 	// 	} else if (tree->fea_types[i] == RANK) {
// 	// 		write_tree.add_split_value()->set_n(tree->split_value[i].n);
// 	// 	} else {
// 	// 		LOG(ERROR) << "Batch data type error.";
// 	// 		return false;
// 	// 	}
// 	// }

//  //  for (auto il : tree->is_leaf) {
//  //  	write_tree.add_is_leaf(il);
//  //  }

// 	// Write the new address book back to disk.
//   std::fstream output(file_name, std::ios::out | std::ios::trunc | std::ios::binary);
//   if (!write_tree.SerializeToOstream(&output)) {
//     LOG(ERROR) << "Failed to write tree.";
//     return false;
//   } else {
//     LOG(INFO) << "Success!";
//   }

//   // Optional:  Delete all global objects allocated by libprotobuf.
//   google::protobuf::ShutdownProtobufLibrary();

//   return true;

// }


// bool write_matrix(MatrixPtr matrixptr, const std::string& file_name) {

//   LOG(INFO) << "Begin write matrix ...";

//   // TODO: 

//   // Verify that the version of the protobuf that we linked against is
//   // compatible with the version of the headers we compiled against.
//   GOOGLE_PROTOBUF_VERIFY_VERSION;

//   dabtree_matrix::Matrix write_matrix;

//   // Read the existing address book.
//   std::fstream input(file_name, std::ios::in | std::ios::binary);
//   if (!input) {
//     LOG(INFO) << file_name << ": File not found. Creating a new file.";
//   } else if (!write_matrix.ParseFromIstream(&input)) {
//     LOG(ERROR) << "Failed to parse matrix data.";
//     return false;
//   }

//   // write tree data into protobuf

//   unsigned int width = matrixptr->GetWidth();
//   unsigned int height = matrixptr->GetHeight();
//   write_matrix.set_width(width);
//   write_matrix.set_height(height);

//   for (size_t i = 0; i < width; ++i) {
//     write_matrix.add_fea_types(dabtree_matrix::FeaType(matrixptr->fea_type(i)));
//   }

//   for (size_t i = 0; i < height; ++i) {
//     for (size_t j = 0; j < width; ++j) {
//       auto &matrix = (*matrixptr);
//       if (matrixptr->fea_type(j) == FeaType::CONT) { 
//         write_matrix.add_data()->set_v(matrix(i, j).v);
//       } else if (matrixptr->fea_type(j) == FeaType::DISC) {
//         write_matrix.add_data()->set_cls(matrix(i, j).cls);
//       } else if (matrixptr->fea_type(j) == FeaType::RANK) {
//         write_matrix.add_data()->set_level(matrix(i, j).level);
//       } else {
//         LOG(ERROR) << "Batch data type error.";
//         return false;
//       }
//     }
//   }

//   // Write the new address book back to disk.
//   std::fstream output(file_name, std::ios::out | std::ios::trunc | std::ios::binary);
//   if (!write_matrix.SerializeToOstream(&output)) {
//     LOG(ERROR) << "Failed to write matrix.";
//     return false;
//   } else {
//     LOG(INFO) << "Success!";
//   }

//   // Optional:  Delete all global objects allocated by libprotobuf.
//   google::protobuf::ShutdownProtobufLibrary();

//   return true;

// }


// int main(int argc, char const *argv[]) {
	
// 	/* test code */
// 	// std::string file_name = "BATCH_DATA_FILE";
// 	// BatchPtr batch_data = std::make_shared<Batch>();;

// 	// batch_data->fea_types.push_back(CONT);
// 	// batch_data->fea_types.push_back(DISC);
// 	// batch_data->fea_types.push_back(RANK);
// 	// std::vector<Value> sample = {{.v=1}, {.cls=0}, {.n=2}};
// 	// batch_data->samples.push_back(sample);
// 	// batch_data->samples.push_back(sample);

// 	// write_batch_data(batch_data, file_name);


// 	std::string tree_file_name = "TREE_DATA_FILE";
// 	RegTreePtr tree = std::make_shared<RegTree>();

// 	tree->fea_types[1] = FeaType::CONT;
//   tree->fea_types[2] = FeaType::DISC;
//   tree->fea_types[3] = FeaType::RANK;
//   tree->split_fea[1] = 0;
//   tree->split_fea[2] = 1;
//   tree->split_fea[3] = 2;
//   tree->is_leaf[1] = false;
//   tree->is_leaf[2] = false;
//   tree->is_leaf[3] = false;
//   tree->split_value[1] = {.v=0.5};
//   tree->split_value[2] = {.cls=1};
//   tree->split_value[3] = {.n=1};
//   tree->split_value[4] = {0.4};
//   tree->split_value[5] = {0.5};
//   tree->split_value[6] = {0.6};
//   tree->split_value[7] = {0.7};

//   write_tree(tree, tree_file_name);

// 	return 0;
// }
