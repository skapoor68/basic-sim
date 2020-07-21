/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Originally based on the FlowScheduler (which is authored by Simon, Hussain)
 */

#include "udp-burst-scheduler.h"

namespace ns3 {

    UdpBurstScheduler::UdpBurstScheduler(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology) {
        printf("UDP BURST SCHEDULER\n");

        m_basicSimulation = basicSimulation;
        m_topology = topology;

        // Check if it is enabled explicitly
        m_enabled = parse_boolean(m_basicSimulation->GetConfigParamOrDefault("enable_udp_burst_scheduler", "false"));
        if (!m_enabled) {
            std::cout << "  > Not enabled explicitly, so disabled" << std::endl;

        } else {
            std::cout << "  > UDP burst scheduler is enabled" << std::endl;

            // Properties we will use often
            m_nodes = m_topology->GetNodes();
            m_simulation_end_time_ns = m_basicSimulation->GetSimulationEndTimeNs();
            m_enable_logging_for_ids = parse_set_positive_int64(m_basicSimulation->GetConfigParamOrDefault("udp_burst_enable_logging_for_ids", "set()"));

            // Distributed run information
            m_system_id = m_basicSimulation->GetSystemId();
            m_enable_distributed = m_basicSimulation->IsDistributedEnabled();
            m_distributed_node_system_id_assignment = m_basicSimulation->GetDistributedNodeSystemIdAssignment();

            // Read schedule
            std::vector<UdpBurstInfo> complete_schedule = read_udp_burst_schedule(
                    m_basicSimulation->GetRunDir() + "/" + m_basicSimulation->GetConfigParamOrFail("udp_burst_schedule_filename"),
                    m_topology,
                    m_simulation_end_time_ns
            );

            // Filter the schedule to only have UDP bursts from a node this system controls
            if (m_enable_distributed) {
                std::vector<UdpBurstInfo> filtered_schedule;
                for (UdpBurstInfo &entry : complete_schedule) {
                    if (m_distributed_node_system_id_assignment[entry.GetFromNodeId()] == m_system_id) {
                        filtered_schedule.push_back(entry);
                    }
                }
                m_schedule = filtered_schedule;
            } else {
                m_schedule = complete_schedule;
            }

            // Schedule read
            printf("  > Read schedule (total UDP bursts: %lu)\n", m_schedule.size());
            m_basicSimulation->RegisterTimestamp("Read UDP burst schedule");

            // Determine filenames
            if (m_enable_distributed) {
                m_udp_bursts_outgoing_csv_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_udp_bursts_outgoing.csv";
                m_udp_bursts_outgoing_txt_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_udp_bursts_outgoing.txt";
                m_udp_bursts_incoming_csv_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_udp_bursts_incoming.csv";
                m_udp_bursts_incoming_txt_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_udp_bursts_incoming.txt";
            } else {
                m_udp_bursts_outgoing_csv_filename = m_basicSimulation->GetLogsDir() + "/udp_bursts_outgoing.csv";
                m_udp_bursts_outgoing_txt_filename = m_basicSimulation->GetLogsDir() + "/udp_bursts_outgoing.txt";
                m_udp_bursts_incoming_csv_filename = m_basicSimulation->GetLogsDir() + "/udp_bursts_incoming.csv";
                m_udp_bursts_incoming_txt_filename = m_basicSimulation->GetLogsDir() + "/udp_bursts_incoming.txt";
            }

            // Remove files if they are there
            remove_file_if_exists(m_udp_bursts_outgoing_csv_filename);
            remove_file_if_exists(m_udp_bursts_outgoing_txt_filename);
            remove_file_if_exists(m_udp_bursts_incoming_csv_filename);
            remove_file_if_exists(m_udp_bursts_incoming_txt_filename);
            printf("  > Removed previous UDP burst log files if present\n");
            m_basicSimulation->RegisterTimestamp("Remove previous UDP burst log files");

            // Install sink on each endpoint node
            std::cout << "  > Setting up UDP burst applications on all endpoint nodes" << std::endl;
            for (int64_t endpoint : m_topology->GetEndpoints()) {
                if (!m_enable_distributed || m_distributed_node_system_id_assignment[endpoint] == m_system_id) {

                    // Setup the application
                    UdpBurstHelper udpBurstHelper(1026);
                    ApplicationContainer app = udpBurstHelper.Install(m_nodes.Get(endpoint));
                    app.Start(Seconds(0.0));
                    m_apps.push_back(app);

                    // Plan all burst originating from there
                    Ptr<UdpBurstApplication> udpBurstApp = app.Get(0)->GetObject<UdpBurstApplication>();
                    for (UdpBurstInfo entry : m_schedule) {
                        if (entry.GetFromNodeId() == endpoint) {
                            udpBurstApp->RegisterBurst(
                                    entry,
                                    InetSocketAddress(m_nodes.Get(entry.GetToNodeId())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1026)
                            );
                        }
                    }

                }
            }
            m_basicSimulation->RegisterTimestamp("Setup UDP burst applications");

        }

        std::cout << std::endl;
    }

    void UdpBurstScheduler::WriteResults() {
        std::cout << "STORE UDP BURST RESULTS" << std::endl;

        // Check if it is enabled explicitly
        if (!m_enabled) {
            std::cout << "  > Not enabled, so no UDP burst results are written" << std::endl;

        } else {

            // Open files
            std::cout << "  > Opening UDP burst log files:" << std::endl;
            FILE* file_outgoing_csv = fopen(m_udp_bursts_outgoing_csv_filename.c_str(), "w+");
            std::cout << "    >> Opened: " << m_udp_bursts_outgoing_csv_filename << std::endl;
            FILE* file_outgoing_txt = fopen(m_udp_bursts_outgoing_txt_filename.c_str(), "w+");
            std::cout << "    >> Opened: " << m_udp_bursts_outgoing_txt_filename << std::endl;
            FILE* file_incoming_csv = fopen(m_udp_bursts_incoming_csv_filename.c_str(), "w+");
            std::cout << "    >> Opened: " << m_udp_bursts_incoming_csv_filename << std::endl;
            FILE* file_incoming_txt = fopen(m_udp_bursts_incoming_txt_filename.c_str(), "w+");
            std::cout << "    >> Opened: " << m_udp_bursts_incoming_txt_filename << std::endl;

            // Header
            std::cout << "  > Writing udp_bursts_{incoming, outgoing}.txt headers" << std::endl;
            fprintf(
                    file_outgoing_txt, "%-16s%-10s%-10s%-16s%-16s%-20s%-22s%-22s%-16s%s\n",
                    "UDP burst ID", "Source", "Target", "Start time", "Duration",
                    "Target rate", "Rate (incl. headers)", "Rate (payload only)", "Packets out", "Metadata"
            );

            // Go over each application
            std::cout << "  > Writing log files" << std::endl;
            for (size_t i = 0; i < m_apps.size(); i++) {
                Ptr<UdpBurstApplication> udpBurstApp = m_apps[i].Get(0)->GetObject<UdpBurstApplication>();

                // Outgoing bursts
                std::vector<std::tuple<UdpBurstInfo, uint64_t>> outgoing_bursts = udpBurstApp->GetOutgoingBurstsInformation();
                for (size_t j = 0; j < outgoing_bursts.size(); j++) {
                    UdpBurstInfo info = std::get<0>(outgoing_bursts[j]);
                    uint64_t sent_out_counter = std::get<1>(outgoing_bursts[j]);

                    // Write plain to the CSV
                    fprintf(
                            file_outgoing_csv, "%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRIu64 ",%s\n",
                            info.GetUdpBurstId(), info.GetFromNodeId(), info.GetToNodeId(), info.GetRateBytePerSec(), info.GetStartTimeNs(),
                            info.GetDurationNs(), sent_out_counter, info.GetMetadata().c_str()
                    );

                    int64_t effective_duration_ns = info.GetStartTimeNs() + info.GetDurationNs() >= m_simulation_end_time_ns ? m_simulation_end_time_ns - info.GetStartTimeNs() : info.GetDurationNs();

                    // Write nicely formatted to the text
                    char str_start_time[100];
                    sprintf(str_start_time, "%.2f ms", nanosec_to_millisec(info.GetStartTimeNs()));
                    char str_duration_ms[100];
                    sprintf(str_duration_ms, "%.2f ms", nanosec_to_millisec(info.GetDurationNs()));
                    char str_target_rate[100];
                    sprintf(str_target_rate, "%.2f Mbit/s", info.GetRateBytePerSec() / 125000.0);
                    char str_eff_rate_incl_headers[100];
                    sprintf(str_eff_rate_incl_headers, "%.2f Mbit/s", byte_to_megabit(sent_out_counter * 1500) / nanosec_to_sec(effective_duration_ns));
                    char str_eff_rate_payload_only[100];
                    sprintf(str_eff_rate_payload_only, "%.2f Mbit/s", byte_to_megabit(sent_out_counter * 1472) / nanosec_to_sec(effective_duration_ns));
                    fprintf(
                            file_outgoing_txt,
                            "%-16" PRId64 "%-10" PRId64 "%-10" PRId64 "%-16s%-16s%-20s%-22s%-22s%-16" PRIu64 "%s\n",
                            info.GetUdpBurstId(),
                            info.GetFromNodeId(),
                            info.GetToNodeId(),
                            str_start_time,
                            str_duration_ms,
                            str_target_rate,
                            str_eff_rate_incl_headers,
                            str_eff_rate_payload_only,
                            sent_out_counter,
                            info.GetMetadata().c_str()
                    );

                }

            }

            // Close files
            std::cout << "  > Closing UDP burst log files:" << std::endl;
            fclose(file_outgoing_csv);
            std::cout << "    >> Closed: " << m_udp_bursts_outgoing_csv_filename << std::endl;
            fclose(file_outgoing_txt);
            std::cout << "    >> Closed: " << m_udp_bursts_outgoing_txt_filename << std::endl;
            fclose(file_incoming_csv);
            std::cout << "    >> Closed: " << m_udp_bursts_incoming_csv_filename << std::endl;
            fclose(file_incoming_txt);
            std::cout << "    >> Closed: " << m_udp_bursts_incoming_txt_filename << std::endl;

            // Register completion
            std::cout << "  > UDP burst log files have been written" << std::endl;
            m_basicSimulation->RegisterTimestamp("Write UDP burst log files");

        }

        std::cout << std::endl;
    }

}
