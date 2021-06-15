/**
 * @file methods/decision_tree/all_categorical_split_impl.hpp
 * @author Ryan Curtin
 *
 * Implementation of the AllCategoricalSplit categorical split class.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_METHODS_DECISION_TREE_ALL_CATEGORICAL_SPLIT_IMPL_HPP
#define MLPACK_METHODS_DECISION_TREE_ALL_CATEGORICAL_SPLIT_IMPL_HPP

namespace mlpack {
namespace tree {

/**
 * Helper function to store split information. This is used for regression.
 * payload contains the information to be stored in splitInfo.
 */
static void StoreSplitInfo(double& splitInfo, const double& payload)
{
  splitInfo = payload;
}

/**
 * Helper function to store split information. This is used for classification.
 * payload contains the information to be stored in splitInfo.
 */
static void StoreSplitInfo(arma::vec& splitInfo, const double& payload)
{
  splitInfo.set_size(1);
  splitInfo[0] = payload;
}

template<typename FitnessFunction>
template<bool UseWeights, typename VecType, typename LabelsType,
         typename WeightVecType, typename SplitInfoType>
double AllCategoricalSplit<FitnessFunction>::SplitIfBetter(
    const double bestGain,
    const VecType& data,
    const size_t numCategories,
    const LabelsType& labels,
    const size_t numClasses,
    const WeightVecType& weights,
    const size_t minimumLeafSize,
    const double minimumGainSplit,
    SplitInfoType& splitInfo,
    AuxiliarySplitInfo& /* aux */)
{
  // Count the number of elements in each potential child.
  const double epsilon = 1e-7; // Tolerance for floating-point errors.
  arma::Col<size_t> counts(numCategories, arma::fill::zeros);

  // If we are using weighted training, learn the weights for each child too.
  arma::vec childWeightSums;
  double sumWeight = 0.0;
  if (UseWeights)
    childWeightSums.zeros(numCategories);

  for (size_t i = 0; i < data.n_elem; ++i)
  {
    counts[(size_t) data[i]]++;

    if (UseWeights)
    {
      childWeightSums[(size_t) data[i]] += weights[i];
      sumWeight += weights[i];
    }
  }

  // If each child will have the minimum number of points in it, we can split.
  // Otherwise we can't.
  if (arma::min(counts) < minimumLeafSize)
    return DBL_MAX;

  // Calculate the gain of the split.  First we have to calculate the labels
  // that would be assigned to each child.
  arma::uvec childPositions(numCategories, arma::fill::zeros);
  std::vector<arma::Row<typename LabelsType::elem_type>> childLabels(numCategories);
  std::vector<arma::Row<double>> childWeights(numCategories);
  for (size_t i = 0; i < numCategories; ++i)
  {
    // Labels and weights should have same length.
    childLabels[i].zeros(counts[i]);
    if (UseWeights)
      childWeights[i].zeros(counts[i]);
  }

  // Extract labels for each child.
  for (size_t i = 0; i < data.n_elem; ++i)
  {
    const size_t category = (size_t) data[i];

    if (UseWeights)
    {
      childLabels[category][childPositions[category]] = labels[i];
      childWeights[category][childPositions[category]++] = weights[i];
    }
    else
    {
      childLabels[category][childPositions[category]++] = labels[i];
    }
  }

  double overallGain = 0.0;
  for (size_t i = 0; i < counts.n_elem; ++i)
  {
    // Calculate the gain of this child.
    const double childPct = UseWeights ?
        double(childWeightSums[i]) / sumWeight :
        double(counts[i]) / double(data.n_elem);
    const double childGain = FitnessFunction::template Evaluate<UseWeights>(
        childLabels[i], numClasses, childWeights[i]);

    overallGain += childPct * childGain;
  }

  if (overallGain > bestGain + minimumGainSplit + epsilon)
  {
    // This is better, so store it in splitInfo and return.
    StoreSplitInfo(splitInfo, numCategories);
    return overallGain;
  }

  // Otherwise there was no improvement.
  return DBL_MAX;
}

template<typename FitnessFunction>
size_t AllCategoricalSplit<FitnessFunction>::NumChildren(
    const double& splitInfo,
    const AuxiliarySplitInfo& /* aux */)
{
  return (size_t) splitInfo;
}

template<typename FitnessFunction>
template<typename ElemType>
size_t AllCategoricalSplit<FitnessFunction>::CalculateDirection(
    const ElemType& point,
    const double& /* splitInfo */,
    const AuxiliarySplitInfo& /* aux */)
{
  return (size_t) point;
}

} // namespace tree
} // namespace mlpack

#endif
