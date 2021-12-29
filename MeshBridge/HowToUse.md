# Step 1

Connect to the Wifi of the Mesh

# Step 2

```
> ip addr

...
2: wlp2s0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default qlen 1000
    link/ether 9c:b6:d0:92:df:4d brd ff:ff:ff:ff:ff:ff
    inet 10.235.233.2/24 brd 10.235.233.255 scope global dynamic noprefixroute wlp2s0
       valid_lft 7188sec preferred_lft 7188sec
    inet6 fe80::6496:4910:8f09:91fd/64 scope link noprefixroute 
       valid_lft forever preferred_lft forever
...
```

Copy the IP after inet of wifi network

# Step 3

```
cd mesh_bridge
cmake .
make
```

# Step 4

Start the Meshbridge with following arguments. Make sure to set the last digit of the ip to 1
```
cd bin
mv ../ChannelDestMapping.csv .    
./painlessMeshBoost --client 10.235.233.1 --prgl 6  
```

# Step 5 (optional)

```
Waiting for nodes...
679078889
679072877
```

Update the nodeIds in ChannelDestMapping.csv 