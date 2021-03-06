#include "AlgorithmMIP.hpp"

#include <sstream>

#include "gurobi_c++.h"

Solution AlgorithmMIP::solveMinCenters(const Graph& graph, int radius)
{
    Solution solution;

    GRBVar* x;
    GRBVar** c;

    try
    {
        std::cout << "Creating the Gurobi environment... ";

        GRBEnv env = GRBEnv(true);
        env.start();

        std::cout << "done.\n";

        std::cout << "Creating the Gurobi model... ";

        GRBModel model = GRBModel(env);

        std::cout << "done.\n";

        std::cout << "Creating the variables... ";

        x = new GRBVar[graph.getNbVertices()];
        for (int vertex = 0; vertex < graph.getNbVertices(); ++vertex)
        {
            std::stringstream ss;
            ss << "x[" << vertex << "]";
            x[vertex] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, ss.str());
        }

        c = new GRBVar*[graph.getNbVertices()];
        for (int vertex1 = 0; vertex1 < graph.getNbVertices(); ++vertex1)
        {
            c[vertex1] = new GRBVar[graph.getNbVertices()];
            for (int vertex2 = 0; vertex2 < graph.getNbVertices(); ++vertex2)
            {
                std::stringstream ss;
                ss << "c[" << vertex1 << "][" << vertex2 << "]";
                c[vertex1][vertex2] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, ss.str());
            }
        }

        std::cout << "done.\n";

        std::cout << "Creating the objective function... ";

        GRBLinExpr obj = 0;
        for (int vertex = 0; vertex < graph.getNbVertices(); ++vertex)
        {
            obj += x[vertex];
        }

        model.setObjective(obj);

        std::cout << "done.\n";

        std::cout << "Creating the constraints... ";

        for (int vertex1 = 0; vertex1 < graph.getNbVertices(); ++vertex1)
        {
            for (int vertex2 = 0; vertex2 < graph.getNbVertices(); ++vertex2)
            {
                model.addConstr(graph.getDistance(vertex1, vertex2) * c[vertex1][vertex2] <= radius);
            }
        }

        for (int vertex1 = 0; vertex1 < graph.getNbVertices(); ++vertex1)
        {
            for (int vertex2 = 0; vertex2 < graph.getNbVertices(); ++vertex2)
            {
                model.addConstr(c[vertex1][vertex2] <= x[vertex1]);
            }
        }

        for (int vertex = 0; vertex < graph.getNbVertices(); ++vertex)
        {
            GRBLinExpr lhs = 0;
            for (int center = 0; center < graph.getNbVertices(); ++center)
            {
                lhs += c[center][vertex];
            }

            model.addConstr(lhs == 1);
        }

        std::cout << "done.\n";

        std::cout << "Configuring solver... ";

        model.set(GRB_DoubleParam_TimeLimit, 600.0);
        model.set(GRB_IntParam_Threads, 24);

        std::cout << "done.\n";

        std::cout << "Running solver... ";

        model.optimize();
        model.write("model.lp");

        std::cout << "done.\n";

        int64_t status = model.get(GRB_IntAttr_Status);
        if (status == GRB_OPTIMAL || (status == GRB_TIME_LIMIT && model.get(GRB_IntAttr_SolCount) > 0))
        {
            solution.isValid = true;

            std::cout << "Success! Status: " << status << ".\n";

            std::cout << "Objective value: " << model.get(GRB_DoubleAttr_ObjVal) << '\n';

            for (int vertex = 0; vertex < graph.getNbVertices(); ++vertex)
            {
                if (x[vertex].get(GRB_DoubleAttr_X) > 0.0001)
                {
                    std::cout << "x[" << vertex << "]: " << x[vertex].get(GRB_DoubleAttr_X) << '\n';
                    solution.centers.push_back(vertex);
                }
            }

            for (int vertex1 = 0; vertex1 < graph.getNbVertices(); ++vertex1)
            {
                for (int vertex2 = 0; vertex2 < graph.getNbVertices(); ++vertex2)
                {
                    if (c[vertex1][vertex2].get(GRB_DoubleAttr_X) > 0.0001)
                    {
                        std::cout << "c[" << vertex1 << "][" << vertex2 << "]: " << c[vertex1][vertex2].get(GRB_DoubleAttr_X) << '\n';
                    }
                }
            }
        }
        else
        {
            std::cout << "Fail! Status: " << status << ".\n";
        }
    }
    catch (GRBException e)
    {
        std::cout << "Gurobi failed with code " << e.getErrorCode() << ".\n";
        std::cout << e.getMessage() << '\n';
    }
    catch (...)
    {
        std::cout << "Gurobi failed with unknown exception.\n";
    }

    return solution;
}

Solution AlgorithmMIP::solveMinRadius(const Graph& graph, int nbCenters)
{
    Solution solution;

    GRBVar r;
    GRBVar* x;
    GRBVar** c;

    try
    {
        std::cout << "Creating the Gurobi environment... ";

        GRBEnv env = GRBEnv(true);
        env.start();

        std::cout << "done.\n";

        std::cout << "Creating the Gurobi model... ";

        GRBModel model = GRBModel(env);

        std::cout << "done.\n";

        std::cout << "Creating the variables... ";

        int maxDist = 0;
        for (int vertex1 = 0; vertex1 < graph.getNbVertices(); ++vertex1)
        {
            for (int vertex2 = vertex1; vertex2 < graph.getNbVertices(); ++vertex2)
            {
                if (graph.getDistance(vertex1, vertex2) > maxDist)
                {
                    maxDist = graph.getDistance(vertex1, vertex2);
                }
            }
        }

        r = model.addVar(0.0, maxDist, 0.0, GRB_INTEGER, "r");

        x = new GRBVar[graph.getNbVertices()];
        for (int vertex = 0; vertex < graph.getNbVertices(); ++vertex)
        {
            std::stringstream ss;
            ss << "x[" << vertex << "]";
            x[vertex] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, ss.str());
        }

        c = new GRBVar * [graph.getNbVertices()];
        for (int vertex1 = 0; vertex1 < graph.getNbVertices(); ++vertex1)
        {
            c[vertex1] = new GRBVar[graph.getNbVertices()];
            for (int vertex2 = 0; vertex2 < graph.getNbVertices(); ++vertex2)
            {
                std::stringstream ss;
                ss << "c[" << vertex1 << "][" << vertex2 << "]";
                c[vertex1][vertex2] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, ss.str());
            }
        }

        std::cout << "done.\n";

        std::cout << "Creating the objective function... ";

        GRBLinExpr obj = r;
        model.setObjective(obj);

        std::cout << "done.\n";

        std::cout << "Creating the constraints... ";

        for (int vertex1 = 0; vertex1 < graph.getNbVertices(); ++vertex1)
        {
            for (int vertex2 = 0; vertex2 < graph.getNbVertices(); ++vertex2)
            {
                model.addConstr(graph.getDistance(vertex1, vertex2) * c[vertex1][vertex2] <= r);
            }
        }

        for (int vertex = 0; vertex < graph.getNbVertices(); ++vertex)
        {
            GRBLinExpr lhs = 0;
            for (int center = 0; center < graph.getNbVertices(); ++center)
            {
                lhs += c[center][vertex];
            }

            model.addConstr(lhs == 1);
        }

        for (int vertex1 = 0; vertex1 < graph.getNbVertices(); ++vertex1)
        {
            for (int vertex2 = 0; vertex2 < graph.getNbVertices(); ++vertex2)
            {
                model.addConstr(c[vertex1][vertex2] <= x[vertex1]);
            }
        }

        GRBLinExpr lhs = 0;
        for (int vertex = 0; vertex < graph.getNbVertices(); ++vertex)
        {
            lhs += x[vertex];
        }

        model.addConstr(lhs <= nbCenters);

        std::cout << "done.\n";

        std::cout << "Configuring solver... ";

        model.set(GRB_DoubleParam_TimeLimit, 600.0);
        model.set(GRB_IntParam_Threads, 24);

        std::cout << "done.\n";

        std::cout << "Running solver... ";

        model.optimize();
        model.write("model.lp");

        std::cout << "done.\n";

        int64_t status = model.get(GRB_IntAttr_Status);
        if (status == GRB_OPTIMAL || (status == GRB_TIME_LIMIT && model.get(GRB_IntAttr_SolCount) > 0))
        {
            solution.isValid = true;

            std::cout << "Success! Status: " << status << ".\n";

            std::cout << "Objective value: " << model.get(GRB_DoubleAttr_ObjVal) << '\n';

            for (int vertex = 0; vertex < graph.getNbVertices(); ++vertex)
            {
                if (x[vertex].get(GRB_DoubleAttr_X) > 0.0)
                {
                    std::cout << "x[" << vertex << "]: " << x[vertex].get(GRB_DoubleAttr_X) << '\n';
                    solution.centers.push_back(vertex);
                }
            }

            for (int vertex1 = 0; vertex1 < graph.getNbVertices(); ++vertex1)
            {
                for (int vertex2 = 0; vertex2 < graph.getNbVertices(); ++vertex2)
                {
                    if (c[vertex1][vertex2].get(GRB_DoubleAttr_X) > 0.0)
                    {
                        std::cout << "c[" << vertex1 << "][" << vertex2 << "]: " << c[vertex1][vertex2].get(GRB_DoubleAttr_X) << '\n';
                    }
                }
            }
        }
        else
        {
            std::cout << "Fail! Status: " << status << ".\n";
        }
    }
    catch (GRBException e)
    {
        std::cout << "Gurobi failed with code " << e.getErrorCode() << ".\n";
        std::cout << e.getMessage() << '\n';
    }
    catch (...)
    {
        std::cout << "Gurobi failed with unknown exception.\n";
    }

    return solution;
}