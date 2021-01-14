/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Georgia Institute of Technology
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
 * Adapted from BulkSendApplication by:
 * Author: George F. Riley <riley@ece.gatech.edu>
 */

#ifndef TCP_FLOW_SEND_APPLICATION_H
#define TCP_FLOW_SEND_APPLICATION_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/string.h"
#include "ns3/traced-callback.h"
#include "ns3/log-update-helper.h"

namespace ns3 {

class Address;
class Socket;

class TcpFlowSendApplication : public Application
{
public:
  static TypeId GetTypeId (void);

  TcpFlowSendApplication ();

  virtual ~TcpFlowSendApplication ();

  int64_t GetAckedBytes();
  Ptr<Socket> GetSocket();
  int64_t GetCompletionTimeNs();
  bool IsCompleted();
  bool IsConnFailed();
  bool IsClosedByError();
  bool IsClosedNormally();
  void FinalizeDetailedLogs();

protected:
  virtual void DoDispose (void);
private:
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  /**
   * Send data until the L4 transmission buffer is full.
   */
  void SendData ();

  Ptr<Socket>     m_socket;       //!< Associated socket
  Address         m_peer;         //!< Peer address
  bool            m_connected;    //!< True if connected
  uint32_t        m_sendSize;     //!< Size of data to send each time
  uint64_t        m_maxBytes;     //!< Limit total number of bytes sent
  uint64_t        m_tcpFlowId;        //!< Flow identifier
  uint64_t        m_totBytes;         //!< Total bytes sent so far
  int64_t         m_completionTimeNs; //!< Completion time in nanoseconds
  bool            m_connFailed;       //!< Whether the connection failed
  bool            m_closedNormally;   //!< Whether the connection closed normally
  bool            m_closedByError;    //!< Whether the connection closed by error
  uint64_t        m_ackedBytes;       //!< Amount of acknowledged bytes cached after close of the socket
  bool            m_isCompleted;      //!< True iff the flow is completed fully AND closed normally
  std::string     m_additionalParameters; //!< Not used in this version of the application
  LogUpdateHelper m_log_update_helper_progress_byte; //!< Progress detailed logging
  LogUpdateHelper m_log_update_helper_rtt_ns;        //!< RTT estimate detailed logging
  LogUpdateHelper m_log_update_helper_rto_ns;        //!< Retransmission time-out detailed logging
  LogUpdateHelper m_log_update_helper_cwnd_byte;     //!< Congestion window detailed logging
  LogUpdateHelper m_log_update_helper_cwnd_inflated_byte; //!< Congestion window inflated detailed logging
  LogUpdateHelper m_log_update_helper_ssthresh_byte; //!< Slow-start threshold detailed logging
  LogUpdateHelper m_log_update_helper_inflight_byte; //!< In-flight detailed logging

  // TCP flow logging
  bool m_enableDetailedLogging;            //!< True iff you want to write detailed logs
  std::string m_baseLogsDir;               //!< Where the logs will be written to:
                                           //!<   logs_dir/tcp_flow_[id]_{progress, cwnd, rtt}.csv
  TracedCallback<Ptr<const Packet> > m_txTrace;

private:
  void ConnectionSucceeded (Ptr<Socket> socket);
  void ConnectionFailed (Ptr<Socket> socket);
  void DataSend (Ptr<Socket>, uint32_t);
  void SocketClosedNormal(Ptr<Socket> socket);
  void SocketClosedError(Ptr<Socket> socket);
  void RttChange (Time, Time newRtt);
  void RtoChange(Time, Time newRto);
  void CwndChange(uint32_t, uint32_t newCwnd);
  void CwndInflatedChange(uint32_t, uint32_t newCwndInflated);
  void SsthreshChange(uint32_t, uint32_t newSsthresh);
  void InflightChange(uint32_t, uint32_t newInflight);

};

} // namespace ns3

#endif /* TCP_FLOW_SEND_APPLICATION_H */
