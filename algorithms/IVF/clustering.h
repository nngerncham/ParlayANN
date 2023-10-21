/* structs which should take a PointRange argument and return a sequence of sequences representing clusters */
#ifndef CLUSTERING_H
#define CLUSTERING_H

#include "parlay/sequence.h"
#include "parlay/primitives.h"
#include "parlay/parallel.h"

#include "../utils/point_range.h"
#include "../HCNNG/clusterEdge.h"
#include "../utils/graph.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <utility>  

template<typename Point, typename PointRange, typename indexType>
using cluster_struct = cluster<Point, PointRange, indexType>;

// using index_type = int32_t;

template<class Point, class PointRange, typename index_type>
struct HCNNGClusterer {
    size_t cluster_size = 1000;
    Graph<index_type> G; // not actually used but needed as a function argument for clusterEdge

    HCNNGClusterer() {}

    HCNNGClusterer(size_t cluster_size) : cluster_size(cluster_size) {}


    // parlay::sequence<parlay::sequence<index_type>> cluster(PointRange points) {
    //     size_t num_points = points.size();
    //     // we make a very sparse sequence to store the clusters
    //     parlay::sequence<std::pair<index_type*, index_type>> clusters(num_points);
    //     // lambda to assign a cluster
    //     auto assign = [&] (Graph<index_type> G, PointRange points, parlay::sequence<index_type>& active_indices, long MSTDeg) {
    //         if (active_indices.size() == 0) { // this should never happen
    //             // std::cout << "HCNNGClusterer::cluster: active_indices.size() == 0" << std::endl;
    //             return;
    //         }
    //         if (std::max_element(active_indices.begin(), active_indices.end())[0] > num_points) {
    //             std::cout << "lambda receiving oversized index" << std::endl;
    //             return;
    //         }
    //         index_type cluster_index = active_indices[0];
    //         // clusters[cluster_index] = parlay::tabulate(cluster_size, [&] (size_t i) {return static_cast<index_type>(active_indices[i]);});
    //         index_type* cluster = new index_type[active_indices.size()];

    //         for (size_t i = 0; i < active_indices.size(); i++) {
    //             cluster[i] = active_indices[i];
    //             if (active_indices[i] > num_points) {
    //                 std::cout << "assign: active_indices[i] > num_points" << std::endl;
    //                 break;
    //             }
    //         }
    //         clusters[cluster_index] = std::make_pair(cluster, active_indices.size());
    //         return;
    //     };

    //     std::cout << "HCNNGClusterer::cluster: calling random_clustering_wrapper" << std::endl;
    //     // should populate the clusters sequence
    //     cluster_struct<Point, PointRange, index_type>().random_clustering_wrapper(G, points, cluster_size, assign, 0);

    //     std::cout << "HCNNGClusterer::cluster: filtering empty clusters" << std::endl;

    //     // remove empty clusters
    //     // clusters = parlay::filter(clusters, [&] (parlay::sequence<index_type> cluster) {return cluster.size() > 0;});
    //     clusters = parlay::filter(clusters, [&] (std::pair<index_type*, index_type> cluster) {return cluster.second > 0;});
    //     parlay::sequence<parlay::sequence<index_type>> result(clusters.size());
    //     for (size_t i = 0; i < clusters.size(); i++) {
    //         result[i] = parlay::sequence<index_type>(clusters[i].first, clusters[i].first + clusters[i].second);
    //     }

    //     // free the memory allocated for the clusters
    //     for (size_t i = 0; i < clusters.size(); i++) {
    //         delete[] clusters[i].first;
    //     }

    //     return result;
    // }


    /* This function should return sequences of sequences describing a partition of `indices`, and should not subsequently require mapping the indices used in the output to their actual values */
    parlay::sequence<parlay::sequence<index_type>> cluster(PointRange points, parlay::sequence<index_type> indices) {
        size_t num_points = indices.size();
        size_t n = points.size();
        // we make a very sparse sequence to store the clusters
        parlay::sequence<std::pair<index_type*, index_type>> clusters = parlay::sequence<std::pair<index_type*, index_type>>(n);
        // lambda to assign a cluster
        auto assign = [&] (Graph<index_type> G, PointRange points, parlay::sequence<index_type>& active_indices, long MSTDeg) {
            if (active_indices.size() == 0) { // this should never happen
                // std::cout << "HCNNGClusterer::cluster: active_indices.size() == 0" << std::endl;
                throw std::runtime_error("HCNNGClusterer::cluster: active_indices.size() == 0");
                return;
            }
            if (std::max_element(active_indices.begin(), active_indices.end())[0] >= n) {
                // std::cout << "lambda receiving oversized index" << std::endl;
                throw std::runtime_error("lambda receiving oversized index");
                return;
            }
            index_type cluster_index = active_indices[0];
            // clusters[cluster_index] = parlay::tabulate(cluster_size, [&] (size_t i) {return static_cast<index_type>(active_indices[i]);});
            index_type* cluster = new index_type[active_indices.size()];

            std::memcpy(cluster, active_indices.begin(), active_indices.size() * sizeof(index_type));

            // for (size_t i = 0; i < active_indices.size(); i++) {
            //     cluster[i] = active_indices[i];
            //     // if (active_indices[i] > n) {
            //     //     // std::cout << "assign: active_indices[i] > num_points" << std::endl;
            //     //     throw std::runtime_error("assign: active_indices[i] > n");
            //     //     break;
            //     // }
            // }
            auto tmp = std::make_pair(cluster, active_indices.size());
            clusters[cluster_index] = tmp;
            return;
        };

        // std::cout << "HCNNGClusterer::cluster: calling random_clustering_wrapper" << std::endl;
        // should populate the clusters sequence
        cluster_struct<Point, PointRange, index_type>().active_indices_rcw(G, points, indices, cluster_size, assign, 0);

        // std::cout << "HCNNGClusterer::cluster: filtering empty clusters" << std::endl;

        // remove empty clusters
        // clusters = parlay::filter(clusters, [&] (parlay::sequence<index_type> cluster) {return cluster.size() > 0;});
        clusters = parlay::filter(clusters, [&] (std::pair<index_type*, index_type> cluster) {return cluster.second > 0;});
        parlay::sequence<parlay::sequence<index_type>> result(clusters.size());

        for (size_t i = 0; i < clusters.size(); i++) {
            result[i] = parlay::sequence<index_type>(clusters[i].first, clusters[i].first + clusters[i].second);
        }

        // free the memory allocated for the clusters
        for (size_t i = 0; i < clusters.size(); i++) {
            delete[] clusters[i].first;
        }

        return result;
    }

    parlay::sequence<parlay::sequence<index_type>> cluster(PointRange points) {
        auto active_indices = parlay::tabulate(points.size(), [&] (index_type i) {return i;});
        return this->cluster(points, active_indices);
    }

};

template<typename T, class Point, typename index_type>
struct KMeansClusterer {
    size_t n_clusters = 1000;

    KMeansClusterer() {}

    KMeansClusterer(size_t n_clusters) : n_clusters(n_clusters) {}

    parlay::sequence<parlay::sequence<index_type>> cluster(PointRange<T, Point> points, parlay::sequence<index_type> indices) {

        size_t num_points = indices.size();
        size_t dim = points.dimension();
        size_t aligned_dim = points.aligned_dimension();
        std::unique_ptr<T[]> centroid_data = std::make_unique<T[]>(n_clusters * aligned_dim);
        auto centroids = parlay::tabulate(n_clusters, [&] (size_t i) {return Point(centroid_data.get() + i * aligned_dim, dim, aligned_dim, i);});

        // initially run HCNNGClusterer to get initial set of clusters
        auto clusters = HCNNGClusterer<Point, PointRange<T, Point>, index_type>(num_points / n_clusters).cluster(points, indices);

        parlay::sort(clusters, [&] (parlay::sequence<index_type> a, parlay::sequence<index_type> b) {return a.size() < b.size();});

        parlay::parallel_for(0, n_clusters, [&] (size_t i) {
            size_t offset = i * aligned_dim;
            parlay::sequence<double> centroid(dim);
            for (size_t j = 0; j < clusters[i].size(); j++){
                for (size_t d = 0; d < dim; d++) {
                    centroid[d] += static_cast<double>(points[clusters[i][j]].get()[d]) / clusters[i].size();
                }
            }
            for (size_t d = 0; d < dim; d++) {
                centroid_data[offset + d] = static_cast<T>(std::round(centroid[d]));
            }
        });

        parlay::sequence<size_t> cluster_assignments = parlay::sequence<size_t>::uninitialized(num_points);

        parlay::parallel_for(0, num_points, [&] (size_t i) {
            double min_dist = std::numeric_limits<double>::max();
            size_t min_index = 0;
            for (size_t j = 0; j < n_clusters; j++) {
                double dist = points[indices[i]].distance(centroids[j]);
                if (dist < min_dist) {
                    min_dist = dist;
                    min_index = j;
                }
            }
            cluster_assignments[i] = min_index;
        });
        
        // now run k-means
        bool not_converged;
        do {
            not_converged = false;

            // compute new centroids
            parlay::parallel_for(0, n_clusters, [&] (size_t i) {
                size_t offset = i * aligned_dim;
                parlay::sequence<double> centroid(dim);
                for (size_t j = 0; j < clusters[i].size(); j++){
                    for (size_t d = 0; d < dim; d++) {
                        centroid[d] += static_cast<double>(points[clusters[i][j]].get()[d]) / clusters[i].size();
                    }
                }
                for (size_t d = 0; d < dim; d++) {
                    centroid_data[offset + d] = static_cast<T>(std::round(centroid[d]));
                }
            });

            // compute new assignments
            parlay::parallel_for(0, num_points, [&] (size_t i) {
            double min_dist = points[indices[i]].distance(centroids[cluster_assignments[i]]);
            size_t min_index = cluster_assignments[i];
            for (size_t j = 0; j < n_clusters; j++) {
                double dist = points[indices[i]].distance(centroids[j]);
                if (dist < min_dist) {
                    min_dist = dist;
                    min_index = j;
                    not_converged = true;
                }
            }
            cluster_assignments[i] = min_index;
        });
        } while (not_converged);

        auto output = parlay::tabulate(num_points, [&] (size_t i) {return std::make_pair(cluster_assignments[i], indices[i]);});

        return parlay::group_by_index(output, n_clusters);
    }
};


#endif // CLUSTERING_H