#include "tree.h"
#include <algorithm>
#include <glog/logging.h>
#include "ThreadPool.h"
#include "thread"
#include <mutex>

std::mutex mu;

bool RegTree::Predict(MatrixPtr batch_ptr, VectorPtr result_ptr) {
  size_t sample_num = batch_ptr->GetHeight();
  if (sample_num == 0)
    return true;
  result_ptr->resize(sample_num, 0.0);
  std::fill(result_ptr->begin(), result_ptr->end(), 0.0);
  if (NumTrees() == 0) {
      return true;
  }

  auto& batch = *batch_ptr;

  ThreadPool pool_(128);
//  std::vector<std::thread> threads;

  for (size_t i = 0; i < sample_num; ++i) {
//      threads.push_back(std::thread([&, this, i](){

      pool_.enqueue([&, this, i]{
      for (size_t j = 0; j < NumTrees(); ++j) {
          size_t cur_node = 1;
          while (true) {
              CHECK_LT(cur_node, MAX_NODE_SIZE) << "Node is out of bound";
              if (split_fea(j, cur_node) == 0) {
                  (*result_ptr)[i] += weight[j] * split_value(j, cur_node).v;
//                  std::cout << i << " " << j << " " << cur_node << " " << split_value(j, cur_node).v << std::endl;
                  break;
                }
              size_t cur_fea = split_fea(j, cur_node);
              Value cur_value = split_value(j, cur_node);
              FeaType cur_type = split_type(cur_fea);
              CHECK_EQ(cur_type, batch.fea_type(cur_fea)) << "types not match";
              cur_node = cur_node*2;
              if (cur_type == FeaType::CONT && batch(i, cur_fea).v >= cur_value.v) {
                  cur_node += 1;
                } else if (cur_type == FeaType::DISC && batch(i, cur_fea).cls == cur_value.cls) {
                  cur_node += 1;
                } else if (cur_type == FeaType::RANK && batch(i, cur_fea).level >= cur_value.level) {
                  cur_node += 1;
                }
            }
        }
      });
    }
//  std::for_each(threads.begin(), threads.end(), mem_fn(&std::thread::join));
  pool_.Join();
//  for (size_t i = 0; i < result_ptr->size(); ++i) std::cout << (*result_ptr)[i] << " ";
  return true;
}



void RegTree::GrowNode(MatrixPtr batch_ptr, node cur_node) {
//  LOG(INFO) << cur_node.row_id << " " << cur_node.row_id << " " << cur_node.low << " " << cur_node.high;
  if (cur_node.col_id >= MAX_NODE_SIZE) return;
  if (cur_node.col_id*2 >= MAX_NODE_SIZE) {
      split_value_->SetValue(cur_node.row_id, cur_node.col_id,
      {.v=batch_ptr->ColMean(0, cur_node.low, cur_node.high)});
      split_fea_->SetValue(cur_node.row_id, cur_node.col_id, {.cls = 0});
      return;
    }
  std::vector<float> best_sses;
  best_sses.resize(batch_ptr->GetWidth(), 0.0);
  std::vector<Value> best_split_values;
  best_split_values.resize(batch_ptr->GetWidth(), {.v=0.0});
  std::vector<size_t> best_split_rows;
  best_split_rows.resize(batch_ptr->GetWidth(), 0);
//  std::vector<std::thread> threads;
  ThreadPool pool_(12);
  for (size_t fea_id = 1; fea_id < batch_ptr->GetWidth(); ++fea_id) {
//      threads.push_back(std::thread([&, batch_ptr, fea_id](){
      pool_.enqueue([&, batch_ptr, fea_id]() {
      float best_sse = -1.0;
      Value best_split_value;
      size_t best_split_row;
      if (batch_ptr->fea_type(fea_id) == FeaType::DISC) {
          std::vector<std::pair<float, size_t>> d;
          for (size_t i = cur_node.low; i < cur_node.high; ++i)
            d.push_back(std::make_pair((*batch_ptr)(i, 0).v, (*batch_ptr)(i, fea_id).cls));
          if (d.size() < 10) return;
          std::sort(d.begin(), d.end(), [](const std::pair<float, size_t>&a, const std::pair<float, size_t>&b){return a.second < b.second;});
          std::vector<size_t> counts;
          std::vector<float> sums;
          std::vector<float> sss;
          std::vector<size_t> splits;
          float count = 1;
          float sum = d[0].first;
          float ss = sum*sum;
          size_t split = d[0].second;
          for (size_t cur_row = 1; cur_row < d.size(); ++cur_row) {
              if (split != d[cur_row].second) {
                  counts.push_back(count);
                  sums.push_back(sum);
                  sss.push_back(ss);
                  splits.push_back(split);
                  count = 0;
                  sum = 0.0;
                  ss = 0.0;
                }
              count += 1;
              float tmp_v = d[cur_row].first;
              sum += tmp_v;
              ss += tmp_v*tmp_v;
              split = d[cur_row].second;
            }
          if (count != 0) {
              counts.push_back(count);
              sums.push_back(sum);
              sss.push_back(ss);
              splits.push_back(split);
            }
          float total_sum = 0.0;
          float total_ss = 0.0;
          size_t total_count = cur_node.high - cur_node.low;
          for (auto& elem : sums)
            total_sum += elem;
          for (auto& elem : sss)
            total_ss += elem;

          for (size_t i = 0; i < counts.size(); ++i) {
              float tmp_mean = sums[i]*1.0/counts[i];
              float tmp = sss[i] - counts[i]*(tmp_mean*tmp_mean);
              tmp_mean = (total_sum - sums[i])*1.0/(total_count - counts[i]);
              tmp += total_ss - sss[i] - (total_count - counts[i]) * tmp_mean * tmp_mean;
              if (best_sse < 0 || tmp < best_sse) {
                  best_sse = tmp;
                  best_split_value.cls = splits[i];
                  best_split_row = cur_node.high - counts[i];
//                  LOG(INFO) << "disc:" << fea_id << " " << best_sse;
                }
            }
          best_sses[fea_id] = best_sse;
          best_split_values[fea_id] = best_split_value;
          best_split_rows[fea_id] = best_split_row;

        } else if (batch_ptr->fea_type(fea_id) == FeaType::CONT) {
//          size_t split_row = cur_node.low + MIN_SAMPLENUM_SPLIT;
//          size_t step = batch_ptr->GetHeight()/MIN_SAMPLENUM_SPLIT;
          size_t step = 10;
          if (cur_node.high - cur_node.low <= step) {
              return;
            }
          std::vector<std::pair<float, float>> d;
          for (size_t i = cur_node.low; i < cur_node.high; ++i)
            d.push_back(std::make_pair((*batch_ptr)(i, 0).v, (*batch_ptr)(i, fea_id).v));
          std::sort(d.begin(), d.end(), [](const std::pair<float, float>&a, const std::pair<float, float>&b){return a.first < b.first;});
          std::vector<size_t> counts;
          std::vector<float> sums;
          std::vector<float> sss;
          std::vector<float> splits;
          float total_count = 0;
          float total_sum = 0.0;
          float total_ss = 0.0;
          float split;
          for (size_t base_row = 0; base_row < d.size(); base_row += step) {
              for (size_t idx = 0; idx < step; ++idx) {
                  size_t cur_row = base_row+idx;
                  if (cur_row >= d.size()) break;
                  total_count += 1;
                  float tmp_v = d[cur_row].first;
                  total_sum += tmp_v;
                  total_ss += tmp_v*tmp_v;
                  split = d[cur_row].second;
                }
                  counts.push_back(total_count);
                  sums.push_back(total_sum);
                  sss.push_back(total_ss);
                  splits.push_back(split);
            }
          for (size_t i = 0; i < counts.size()-1; ++i) {

              float tmp_mean = sums[i]*1.0/counts[i];
              float tmp = sss[i] - counts[i]*(tmp_mean*tmp_mean);
              tmp_mean = (total_sum - sums[i])*1.0/(total_count - counts[i]);
              tmp += total_ss - sss[i] - (total_count - counts[i]) * tmp_mean * tmp_mean;
              if (best_sse < 0 || tmp < best_sse) {
                  best_sse = tmp;
                  best_split_row = counts[i] + 1 + cur_node.low;
                  best_split_value.v = d[best_split_row].second;
//                  LOG(INFO) << "cont:" << fea_id << " " << best_sse;
                }
            }
          best_sses[fea_id] = best_sse;
          best_split_values[fea_id] = best_split_value;
          best_split_rows[fea_id] = best_split_row;

//          while (split_row < cur_node.high) {
//              float cur_sse = batch.SSE(cur_node.low, split_row) + batch.SSE(split_row, cur_node.high);
//              if (best_fea == 0 || cur_sse < best_sse) {
//                  best_sse = cur_sse;
//                  best_fea = fea_id;
//                  best_split_value.v = batch(split_row, fea_id).v;
//                }
//              split_row += MIN_SAMPLENUM_SPLIT;
//            }
        } else if (batch_ptr->fea_type(fea_id) == FeaType::RANK) {
          return;
//          size_t split_row = cur_node.low;
//          int cur_value = (*batch_ptr)(split_row, fea_id).level;
//          while (true) {
//              while (split_row < cur_node.high && cur_value == (*batch_ptr)(split_row, fea_id).level) split_row++;
//              if (split_row == cur_node.high) break;
//              cur_value = (*batch_ptr)(split_row, fea_id).level;
//              float cur_sse = batch_ptr->SSE(cur_node.low, split_row) + batch_ptr->SSE(split_row, cur_node.high);
//              if (best_fea == 0 || cur_sse < best_sse) {
//                  best_sse = cur_sse;
//                  best_fea = fea_id;
//                  best_split_value.level = cur_value;
//                }
//            }
        }

        }); // thread
    }// for
  pool_.Join();

  float best_sse = -1.0;
  Value best_split_value;
  size_t best_split_row;
  size_t best_fea = 0;
  for (size_t i = 1; i < best_sses.size(); ++i) {
      if (best_sses[i] >= 0.0 &&  best_sses[i] < best_sse) {
          best_sse = best_sses[i];
          best_split_value = best_split_values[i];
          best_split_row = best_split_rows[i];
          best_fea = i;
        }
    }

//  LOG(INFO) << "start final";
//  std::for_each(threads.begin(), threads.end(), mem_fn(&std::thread::join));
  float non_split_sse = batch_ptr->SSE(cur_node.low, cur_node.high);
  if (best_fea == 0 || non_split_sse < best_sse) {
      split_value_->SetValue(cur_node.row_id, cur_node.col_id,
      {.v=batch_ptr->ColMean(0, cur_node.low, cur_node.high)});
      split_fea_->SetValue(cur_node.row_id, cur_node.col_id, {.cls = 0});
      return;
    }
  split_fea_->SetValue(cur_node.row_id, cur_node.col_id, {.cls = best_fea});
  split_value_->SetValue(cur_node.row_id, cur_node.col_id, best_split_value);

  size_t mid = best_split_row;
//  LOG(INFO) << cur_node.low << " " << mid << " " << cur_node.high;
  if (batch_ptr->fea_type(best_fea) == FeaType::CONT) {
      batch_ptr->Sort(best_fea, cur_node.low, cur_node.high);
//      while(mid < cur_node.high && (*batch_ptr)(mid, best_fea).v < best_split_value.v) mid++;
    } else if (batch_ptr->fea_type(best_fea) == FeaType::DISC) {
      batch_ptr->Sort(best_fea, cur_node.low, cur_node.high, best_split_value.cls);
//      mid = batch_ptr->Split(best_fea, cur_node.low, cur_node.high, best_split_value.cls);
//      for (size_t i = cur_node.high-1; i >= cur_node.low; --i) {
//        if ((*batch_ptr)(i, best_fea).cls != best_split_value.cls) {
//            mid=i+1;
//            break;
//          }
//      }
//      while(mid < cur_node.high && (*batch_ptr)(mid, best_fea).cls != best_split_value.cls) mid++;
    }

//  else if (batch_ptr->fea_type(best_fea) == FeaType::RANK) {
////      mu.lock();
//      batch_ptr->Sort(best_fea, cur_node.low, cur_node.high);
////      mu.unlock();
//      while((*batch_ptr)(mid, best_fea).level < best_split_value.level) mid++;
//    }

  node left = {.row_id=cur_node.row_id, .col_id=cur_node.col_id*2, .low = cur_node.low, .high=mid};
  node right = {.row_id=cur_node.row_id, .col_id=cur_node.col_id*2+1, .low = mid, .high=cur_node.high};

//  LOG(INFO) << "start thread";

//  ThreadPool pool2_(2);

//  pool2_.enqueue([&]() {
  std::thread t1([this, batch_ptr, left](){GrowNode(batch_ptr, left);});

//  GrowNode(batch_ptr, left);
//    });
//  pool2_.enqueue([&]() {

  GrowNode(batch_ptr, right);
//  std::thread t2([&, this, batch_ptr, right](){GrowNode(batch_ptr, right);});
  t1.join();

//  t2.join();
//    });
//  pool2_.Join();
//  LOG(INFO) << "end";

  return;
}

void RegTree::TrainOneTree(MatrixPtr batch_ptr, float weight) {
  if (split_value_->Empty()) {
      split_value_->SetType(batch_ptr->fea_types());
    } else {
      for (size_t i = 0; i < batch_ptr->GetWidth(); ++i) {
        CHECK_EQ(batch_ptr->fea_type(i), split_type(i));
        }
    }
  AddOneTree(weight);
  size_t row_id = NumTrees()-1;
  node root = {.row_id=row_id, .col_id=1, .low = 0, .high=batch_ptr->GetHeight()};
  GrowNode(batch_ptr, root);
}



void RegTree::PrintOneTree(size_t tree_id, size_t start) {
  if (split_fea(tree_id, start) == 0) {
      printf("{%lu, %f}", split_fea(tree_id, start), split_value(tree_id, start).v);
      return;
    }
  auto type = GetType(split_fea(tree_id, start));
  if (type == FeaType::CONT)
    printf("{%lu, %f}, [", split_fea(tree_id, start), split_value(tree_id, start).v);
  else if (type == FeaType::DISC)
    printf("{%lu, %lu}, [", split_fea(tree_id, start), split_value(tree_id, start).cls);
  else if (type == FeaType::RANK)
    printf("{%lu, %d}, [", split_fea(tree_id, start), split_value(tree_id, start).level);
  PrintOneTree(tree_id, start*2);
  printf(",");
  PrintOneTree(tree_id, start*2+1);
  printf("]");
}

void RegTree::Print() {
  for (size_t i = 0; i < NumTrees(); ++i) {
      printf("%f,", weight[i]);
      PrintOneTree(i, 1);
      printf("\n");
    }
}

