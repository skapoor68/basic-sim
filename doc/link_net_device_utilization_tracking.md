# Link net-device utilization tracking

This tracks the utilization of point-to-point network devices (part of a 'link').

It encompasses the following files:

* `model/core/net-device-utilization-tracker.cc/h` - 
  Link net-device utilization tracker
* `helper/core/ptop-link-net-device-utilization-tracking.cc/h` - 
  Helper to install the utilization trackers on the network devices in a topology.

You can use the application(s) separately, or make use of the helper 
which requires a topology (which is recommended).


## Getting started: using helper

1. Add the following to the `config_ns3.properties` in your run folder (for tracking utilization at a 100ms granularity):

   ```
   enable_link_net_device_utilization_tracking=true
   link_net_device_utilization_tracking_interval_ns=100000000
   link_net_device_utilization_tracking_enable_for_links=all
   ```

2. In your code, import the helper:

   ```
   #include "ns3/ptop-link-net-device-utilization-tracking.h"
   ```
   
3. Before the start of the simulation run, in your code add:

   ```c++
   // Install link net-device utilization trackers
   PtopLinkNetDeviceUtilizationTracking netDeviceUtilizationTracking = PtopLinkNetDeviceUtilizationTracking(basicSimulation, topology);
   ```

4. After the run, in your code add:

   ```c++
   // Write link net-device utilization result
   netDeviceUtilizationTracking.WriteResults();
   ```
   
5. After the run, you should have the link net-device utilization log files in the `logs_ns3` of your run folder.


## Getting started: directly using tracker

1. In your code, import the tracker:

   ```
   #include "ns3/net-device-utilization-tracker.h"
   ```
   
2. Before the start of the simulation run, in your code add:

   ```c++
   Ptr<PointToPointNetDevice> networkDevice = ... // Get the network device from somewhere
   int64_t utilization_interval_ns = 100000000; // 100ms
   Ptr<NetDeviceUtilizationTracker> tracker = CreateObject<NetDeviceUtilizationTracker>(networkDevice, utilization_interval_ns);
   // ... store the tracker to keep it alive and later retrieve its results
   ```

3. After the run, in your code add:

   ```c++
   const std::vector<std::tuple<int64_t, int64_t, int64_t>> intervals = tracker->FinalizeUtilization();
   for (size_t j = 0; j < intervals.size(); j++) {
       int64_t interval_start_ns = std::get<0>(intervals[j]);
       int64_t interval_end_ns = std::get<1>(intervals[j]);
       int64_t interval_busy_time_ns = std::get<2>(intervals[j]);
       double interval_utilization = ((double) interval_busy_time_ns) / (double) (interval_end_ns - interval_start_ns);
       // ... then do something with it, print it
   }
   ```


## Helper information

You MUST set the following key in `config_ns3.properties` for utilization tracking to be enabled:

* `enable_link_net_device_utilization_tracking` : True iff link net-device utilization tracking on all links 
  should be enabled (boolean value, either `true` or `false`)
  
* `link_net_device_utilization_tracking_interval_ns` : The aggregation interval, this effectively 
  determines the granularity of the utilization results. Setting it too low will cause a 
  lot of overhead, setting it too high will make the results less fine. (positive integer
  value in nanoseconds)


The following CAN be set:

* `link_net_device_utilization_tracking_enable_for_links` : 
  Select which links utilization tracking should be enabled (either `all` or set of links 
  (directed edges) `set(a->b, ...)`)

**The link net-device utilization log files**

There are two log files generated by the run in the `logs_ns3` folder within the run folder:

* `link_net_device_utilization.csv` : 
  CSV formatted utilization at the interval level. 
  The format is as follows:

  ```
   <from>,<to>,<interval start (ns)>,<interval end (ns)>,<amount of busy in this interval (ns)>
  ```

* `link_net_device_utilization_compressed.csv` : 
  CSV formatted utilization at the interval level, but adjacent intervals with the same 
  utilization (approximately) are joined together. Particularly handy for plotting. 
  The format is as follows:

   ```
   <from>,<to>,<interval start (ns)>,<interval end (ns)>,<amount of busy in this interval (ns)>
   ```

* `link_net_device_utilization_compressed.txt` : 
  Human readable table showing utilization at the interval level, 
  but compressing together the adjacent intervals which have the same utilization (approximately).

* `link_net_device_utilization_summary.txt` : 
  Human readable table of the mean utilization of each link.