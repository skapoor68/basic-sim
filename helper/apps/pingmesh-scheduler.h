/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 ETH Zurich
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
 */

#ifndef PINGMESH_SCHEDULER_H
#define PINGMESH_SCHEDULER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/basic-simulation.h"
#include "ns3/exp-util.h"
#include "ns3/topology.h"
#include "ns3/udp-rtt-helper.h"

namespace ns3 {

class PingmeshScheduler
{

public:
    PingmeshScheduler(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology);
    void WriteResults();

protected:
    Ptr<BasicSimulation> m_basicSimulation;
    int64_t m_simulation_end_time_ns;
    Ptr<Topology> m_topology = nullptr;
    bool m_enabled;

    NodeContainer m_nodes;
    std::vector<ApplicationContainer> m_apps;
    int64_t m_interval_ns;
    std::vector<std::pair<int64_t, int64_t>> m_pingmesh_endpoint_pairs;
    uint32_t m_system_id;
    bool m_enable_distributed;
    std::vector<int64_t> m_distributed_node_system_id_assignment;
    std::string m_pingmesh_csv_filename;
    std::string m_pingmesh_txt_filename;
};

}

#endif /* PINGMESH_SCHEDULER_H */
