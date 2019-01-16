/**
 * @file range_search_test.cpp
 * @author Niteya Shah
 *
 * Test mlpackMain() of range_search_main.cpp.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#include <string>

#define BINDING_TYPE BINDING_TYPE_TEST
static const std::string testName = "RangeSearchMain";

#include <mlpack/core.hpp>
#include <mlpack/core/util/mlpack_main.hpp>
#include "test_helper.hpp"
#include <mlpack/methods/range_search/range_search_main.cpp>

#include <boost/test/unit_test.hpp>
#include "../test_tools.hpp"

using namespace mlpack;

struct RangeSearchTestFixture
{
 public:

  RangeSearchTestFixture()
  {
    // Cache in the options for this program.
    CLI::RestoreSettings(testName);
  }
  ~RangeSearchTestFixture()
  {
    // Clear the settings.
    bindings::tests::CleanMemory();
    CLI::ClearSettings();
  }
};

BOOST_FIXTURE_TEST_SUITE(RangeSearchMainTest, RangeSearchTestFixture);
/*
* Check that the correct output is returned for a small synthetic input
* case
*/
BOOST_AUTO_TEST_CASE(RangeSearchTest)
{
  //Matrix Input is expected in this format
  arma::mat x = {{0, 3, 3, 4, 3, 1},
                 {4, 4, 4, 5, 5, 2},
                 {0, 1, 2, 2, 3, 3}};
  std::string distancefile = "distances.csv";
  std::string neighborsfile = "neighbors.csv";
  double minv = 0, maxv = 3;
  vector<vector<size_t>> neighborval = {{},
                                        {2, 3, 4},
                                        {1, 3, 4, 5},
                                        {1, 2, 4},
                                        {1, 2, 3},
                                        {2}};
  vector<vector<double>> distanceval = {{},{1, 1.73205, 2.23607},
                                        {1, 1.41421, 1.41421, 3},
                                        {1.73205, 1.41421, 1.41421},
                                        {2.23607, 1.41421, 1.41421},
                                        {3}};
  vector<vector<size_t>> neighbors;
  vector<vector<double>> distances;
  SetInputParam("reference", move(x));
  //To prevent warning for lack of definition
  SetInputParam("min", minv);
  SetInputParam("max", maxv);
  SetInputParam("distances_file", distancefile);
  SetInputParam("neighbors_file", neighborsfile);

  mlpackMain();

  neighbors = ReadData<size_t>(neighborsfile);
  distances = ReadData<double>(distancefile);

  CheckMatrices(neighbors, neighborval, 1e-5f);
  CheckMatrices(distances, distanceval);
}
//Perform a Test for Search with a Query
BOOST_AUTO_TEST_CASE(RangeSeachTestwithQuery)
{
  arma::mat querydata = {{5, 3, 1},{4, 2, 4},{3, 1, 7}};
  arma::mat x = {{0, 3, 3, 4, 3, 1},
                 {4, 4, 4, 5, 5, 2},
                 {0, 1, 2, 2, 3, 3}};
  vector<vector<double>> distanceval = {
                {2.82843, 2.23607, 1.73205, 2.23607, 4.47214},
                {3.74166, 2, 2.23607, 3.31662, 3.60555, 2.82843},
                {4.58258, 4.47214}};
  vector<vector<size_t>> neighborval = {{1,2,3,4,5},{0,1,2,3,4,5},{4,5}};
  vector<vector<size_t>> neighbors;
  vector<vector<double>> distances;
  string distancefile = "distances.csv";
  string neighborsfile = "neighbors.csv";
  double minv = 0, maxv = 5;

  SetInputParam("query", querydata);
  SetInputParam("reference", move(x));
  SetInputParam("min", minv);
  SetInputParam("max", maxv);
  SetInputParam("distances_file", distancefile);
  SetInputParam("neighbors_file", neighborsfile);

  mlpackMain();


  neighbors = ReadData<size_t>(neighborsfile);
  distances = ReadData<double>(distancefile);

  CheckMatrices(neighbors, neighborval, 1e-5f);
  CheckMatrices(distances, distanceval);

}

//check if an output model can be used again for a different usage
BOOST_AUTO_TEST_CASE(ModelCheck)
{
  arma::mat inputdata, querydata;
  double minv = 0, maxv = 3;
  string distancefile = "distances.csv";
  string neighborfile = "neighbors.csv";
  vector<vector<size_t>> neighbors,neighborstemp;
  vector<vector<double>> distances,distancetemp;

  if (!data::Load("iris.csv", inputdata))
    BOOST_FAIL("Unable to load dataset iris.csv!");
  if (!data::Load("iris_test.csv", querydata))
    BOOST_FAIL("Unable to load dataset iris_test.csv!");

  SetInputParam("reference", move(inputdata));
  SetInputParam("min", minv);
  SetInputParam("max", maxv);
  SetInputParam("distances_file", distancefile);
  SetInputParam("neighbors_file", neighborfile);
  SetInputParam("query", querydata);

  mlpackMain();

  neighbors = ReadData<size_t>(neighborfile);
  distances = ReadData<double>(distancefile);

  RSModel* outputmodel = move(CLI::GetParam<RSModel*>("output_model"));
  CLI::GetSingleton().Parameters()["reference"].wasPassed=false;

  SetInputParam("input_model", move(outputmodel));
  SetInputParam("query", move(querydata));

  mlpackMain();

  neighborstemp = ReadData<size_t>(neighborfile);
  distancetemp = ReadData<double>(distancefile);

  CheckMatrices(neighbors, neighborstemp, 1e-5f);
  CheckMatrices(distances, distancetemp);

  if (!( outputmodel == CLI::GetParam<RSModel*>("output_model") ))
  {

    BOOST_FAIL("Models are not Equal");
  }

}

BOOST_AUTO_TEST_CASE(LeafValueTesting)
{
  //Testing 3 different leaf values - default 20, 15 and 25
  //Ensure that results match for different leaf values
  arma::mat inputdata;
  if (!data::Load("iris.csv", inputdata))
    BOOST_FAIL("Unable to load dataset iris.csv!");
  string distancefile = "distances.csv";
  string neighborsfile = "neighbors.csv";
  double minv = 0, maxv = 3;
  vector<vector<size_t>> neighbors, neighborstemp;
  vector<vector<double>> distances, distancestemp;
  vector<int> arr{20, 15, 25};
  SetInputParam("reference", inputdata);
  //To prevent warning for lack of definition
  SetInputParam("min", minv);
  SetInputParam("max", maxv);
  SetInputParam("distances_file", distancefile);
  SetInputParam("neighbors_file", neighborsfile);
  SetInputParam("leaf_size", arr[0]);
  //Default leaf size is 20

  mlpackMain();

  RSModel* outputmodel1 = CLI::GetParam<RSModel*>("output_model");
  neighbors = ReadData<size_t>(neighborsfile);
  distances = ReadData<double>(distancefile);


  for(size_t i = 1 ; i < arr.size() ; i++)
  {
    SetInputParam("leaf_size", arr[i]);
    SetInputParam("reference", inputdata);
    SetInputParam("min", minv);
    SetInputParam("max", maxv);
    SetInputParam("distances_file", distancefile);
    SetInputParam("neighbors_file", neighborsfile);

    mlpackMain();

    neighborstemp = ReadData<size_t>(neighborsfile);
    distancestemp = ReadData<double>(distancefile);

    CheckMatrices(neighbors, neighborstemp, 1e-5f);
    CheckMatrices(distances, distancestemp);

    BOOST_REQUIRE_EQUAL(CheckModelSerial(outputmodel1,
                                   CLI::GetParam<RSModel*>("output_model")), 0);
  }
}

//All trees should give the same results for fixed input parameters
BOOST_AUTO_TEST_CASE(TreeTypeTesting)
{
  string distancefile = "distances.csv";
  string neighborsfile = "neighbors.csv";
  double minv = 0, maxv = 3;
  arma::mat querydata, inputdata;
  vector<vector<size_t>> neighbors, neighborstemp;
  vector<vector<double>> distances, distancestemp;
  vector<string> trees = {"kd", "cover", "r", "r-star", "ball", "x",
                          "hilbert-r", "r-plus", "r-plus-plus", "vp","rp",
                          "max-rp", "ub", "oct"};

  if (!data::Load("iris.csv", inputdata))
    BOOST_FAIL("Unable to load dataset iris.csv!");
  if (!data::Load("iris_test.csv", querydata))
    BOOST_FAIL("Unable to load dataset iris_test.csv!");

  //Define Base with kd Tree
  SetInputParam("tree_type", trees[0]);
  SetInputParam("min", minv);
  SetInputParam("max", maxv);
  SetInputParam("distances_file", distancefile);
  SetInputParam("neighbors_file", neighborsfile);
  SetInputParam("reference", inputdata);
  SetInputParam("query", querydata);

  mlpackMain();

  neighbors = ReadData<size_t>(neighborsfile);
  distances = ReadData<double>(distancefile);
  RSModel* outputmodel1=CLI::GetParam<RSModel*>("output_model");

  for (size_t i = 1;i < trees.size() ; i++)
  {
    if (!data::Load("iris.csv", inputdata))
      BOOST_FAIL("Unable to load dataset iris.csv!");
    if (!data::Load("iris_test.csv", querydata))
      BOOST_FAIL("Unable to load dataset iris_test.csv!");

    SetInputParam("min", minv);
    SetInputParam("max", maxv);
    SetInputParam("distances_file", distancefile);
    SetInputParam("neighbors_file", neighborsfile);
    SetInputParam("query", querydata);
    SetInputParam("reference", inputdata);
    SetInputParam("tree_type", trees[i]);

    mlpackMain();

    neighborstemp = ReadData<size_t>(neighborsfile);
    distancestemp = ReadData<double>(distancefile);

    CheckMatrices(neighbors, neighborstemp);
    CheckMatrices(distances, distancestemp);
    BOOST_REQUIRE_EQUAL(CheckModelSerial(outputmodel1,
                                   CLI::GetParam<RSModel*>("output_model")), 0);
  }
}

BOOST_AUTO_TEST_CASE(RandomBasisTesting)
{
  string distancefile = "distances.csv";
  string neighborsfile = "neighbors.csv";
  double minv = 0, maxv = 3;
  arma::mat querydata, inputdata;
  vector<vector<size_t>> neighbors, neighborstemp;
  vector<vector<double>> distances, distancestemp;
  if (!data::Load("iris.csv", inputdata))
    BOOST_FAIL("Unable to load dataset iris.csv!");
  if (!data::Load("iris_test.csv", querydata))
    BOOST_FAIL("Unable to load dataset iris_test.csv!");

  SetInputParam("min", minv);
  SetInputParam("max", maxv);
  SetInputParam("distances_file", distancefile);
  SetInputParam("neighbors_file", neighborsfile);
  SetInputParam("reference", inputdata);

  mlpackMain();

  RSModel* outputmodel = move(CLI::GetParam<RSModel*>("output_model"));

  SetInputParam("min", minv);
  SetInputParam("max", maxv);
  SetInputParam("distances_file", distancefile);
  SetInputParam("neighbors_file", neighborsfile);
  SetInputParam("reference", inputdata);
  SetInputParam("random_basis",true);

  mlpackMain();

  BOOST_REQUIRE_EQUAL(CheckModelSerial(outputmodel,
                                    CLI::GetParam<RSModel*>("output_model")),0);
}
BOOST_AUTO_TEST_CASE(NaiveModeTest)
{
  string distancefile = "distances.csv";
  string neighborsfile = "neighbors.csv";
  double minv = 0, maxv = 3;
  arma::mat querydata, inputdata;
  vector<vector<size_t>> neighbors, neighborstemp;
  vector<vector<double>> distances, distancestemp;
  if (!data::Load("iris.csv", inputdata))
    BOOST_FAIL("Unable to load dataset iris.csv!");
  if (!data::Load("iris_test.csv", querydata))
    BOOST_FAIL("Unable to load dataset iris_test.csv!");

  SetInputParam("min", minv);
  SetInputParam("max", maxv);
  SetInputParam("distances_file", distancefile);
  SetInputParam("neighbors_file", neighborsfile);
  SetInputParam("reference", inputdata);

  mlpackMain();

  RSModel* outputmodel = move(CLI::GetParam<RSModel*>("output_model"));

  SetInputParam("min", minv);
  SetInputParam("max", maxv);
  SetInputParam("distances_file", distancefile);
  SetInputParam("neighbors_file", neighborsfile);
  SetInputParam("reference", inputdata);
  SetInputParam("naive", true);

  mlpackMain();

  BOOST_REQUIRE_EQUAL(CheckModelSerial(outputmodel,
                                   CLI::GetParam<RSModel*>("output_model")), 0);
}

BOOST_AUTO_TEST_CASE(SingleModeTest)
{
  string distancefile = "distances.csv";
  string neighborsfile = "neighbors.csv";
  double minv = 0, maxv = 3;
  arma::mat querydata, inputdata;
  vector<vector<size_t>> neighbors, neighborstemp;
  vector<vector<double>> distances, distancestemp;
  if (!data::Load("iris.csv", inputdata))
    BOOST_FAIL("Unable to load dataset iris.csv!");
  if (!data::Load("iris_test.csv", querydata))
    BOOST_FAIL("Unable to load dataset iris_test.csv!");

  SetInputParam("min", minv);
  SetInputParam("max", maxv);
  SetInputParam("distances_file", distancefile);
  SetInputParam("neighbors_file", neighborsfile);
  SetInputParam("reference", inputdata);

  mlpackMain();

  RSModel* outputmodel = move(CLI::GetParam<RSModel*>("output_model"));

  SetInputParam("min", minv);
  SetInputParam("max", maxv);
  SetInputParam("distances_file", distancefile);
  SetInputParam("neighbors_file", neighborsfile);
  SetInputParam("reference", inputdata);
  SetInputParam("single_mode", true);

  mlpackMain();

  BOOST_REQUIRE_EQUAL(CheckModelSerial(outputmodel,
                                   CLI::GetParam<RSModel*>("output_model")), 0);
}

BOOST_AUTO_TEST_SUITE_END();
