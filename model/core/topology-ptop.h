/*
 * Copyright (c) 2019 ETH Zurich
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Simon
 * Originally based on, but since heavily adapted/extended, the scratch/main authored by Hussain.
 */

#ifndef TOPOLOGY_PTOP_H
#define TOPOLOGY_PTOP_H

#include <utility>
#include "ns3/core-module.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/topology.h"
#include "ns3/exp-util.h"
#include "ns3/basic-simulation.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/command-line.h"
#include "ns3/traffic-control-helper.h"

namespace ns3 {

class TopologyPtop : public Topology
{
public:

    // Constructors
    static TypeId GetTypeId (void);
    TopologyPtop(Ptr<BasicSimulation> basicSimulation, const Ipv4RoutingHelper& ipv4RoutingHelper);

    // Accessors
    const NodeContainer& GetNodes();
    int64_t GetNumNodes();
    int64_t GetNumUndirectedEdges();
    const std::set<int64_t>& GetSwitches();
    const std::set<int64_t>& GetSwitchesWhichAreTors();
    const std::set<int64_t>& GetServers();
    bool IsValidEndpoint(int64_t node_id);
    const std::set<int64_t>& GetEndpoints();
    const std::vector<std::pair<int64_t, int64_t>>& GetUndirectedEdges();
    const std::set<std::pair<int64_t, int64_t>>& GetUndirectedEdgesSet();
    const std::vector<std::set<int64_t>>& GetAllAdjacencyLists();
    const std::set<int64_t>& GetAdjacencyList(int64_t node_id);
    int64_t GetWorstCaseRttEstimateNs();
    const std::vector<std::pair<uint32_t, uint32_t>>& GetInterfaceIdxsForEdges();

private:

    Ptr<BasicSimulation> m_basicSimulation;

    // Construction
    void ReadRelevantConfig();
    void ReadTopology();
    void SetupNodes(const Ipv4RoutingHelper& ipv4RoutingHelper);
    void SetupLinks();

    // Configuration properties
    double m_link_data_rate_megabit_per_s;
    int64_t m_link_delay_ns;
    int64_t m_link_max_queue_size_pkts;
    int64_t m_worst_case_rtt_ns;
    bool m_disable_qdisc_endpoint_tors_xor_servers;
    bool m_disable_qdisc_non_endpoint_switches;

    // Distributed
    bool m_enable_distributed;
    int64_t m_distributed_logical_processes_k;
    std::vector<int64_t> m_distributed_node_logical_process_assignment;

    // Graph properties
    int64_t m_num_nodes;
    int64_t m_num_undirected_edges;
    std::set<int64_t> m_switches;
    std::set<int64_t> m_switches_which_are_tors;
    std::set<int64_t> m_servers;
    std::vector<std::pair<int64_t, int64_t>> m_undirected_edges;
    std::set<std::pair<int64_t, int64_t>> m_undirected_edges_set;
    std::vector<std::set<int64_t>> m_adjacency_list;
    bool m_has_zero_servers;

    // From generating ns3 objects
    NodeContainer m_nodes;
    std::vector<std::pair<uint32_t, uint32_t>> m_interface_idxs_for_edges;

};

}

#endif //TOPOLOGY_PTOP_H