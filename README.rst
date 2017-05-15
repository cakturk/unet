uNet
====
Just another toy userspace TCP/IP stack.

Compiling
---------
This project is only known to work on Linux systems, although it might
not be so hard to get it working on BSD systems.

In case you do not have access to a Linux system, you can use the
Vagrantfile provided within the project source tree. Using Vagrant,
getting a Linux box with all the dependencies installed and properly
configured is just a breeze. Just issue the following commands::

  $ vagrant up
  # change your working directory to the root of the source tree
  $ cd /vagrant_data
  $ make
  $ make check

If you don't want to use Vagrant for some reason, here are the
dependencies you are going to need.

- Make sure you have ``make`` and ``gcc`` available on your system.
- Just do a ``make`` to compile the entire application.
- Install ``scapy`` packet manipulation tool.
- You can run tests by issuing a ``make check``

How to use?
-----------
``unet`` features an interactive shell to communicate with the stack.
The uNet shell itself comes with a bunch of commands that are similar to
the UNIX equivalents::

  [vagrant@unet-devel vagrant_data]$ ./unet
  iface: tap0, hwaddr: 56:85:6f:7f:a0:c1, ipaddr: 172.28.128.44
  unet-shell-> help
  ip         - set/get interface IP address
  hwaddr     - set/get mac address
  route      - show/manipulate the routing tables
  nc         - arbitrary TCP and UDP connections and listens

To change IP and mac address for the virtual interface::

  unet-shell-> hwaddr set aa:bb:cc:11:22:33
  unet-shell-> ip set 192.168.1.206/24

Set up a default route with route command.::

  unet-shell-> route setgw 10.10.20.24

nc command is very similar to the regular ``netcat`` tool that is widely
used in the UNIX community::

   unet-shell-> nc -u 192.168.1.100 5432

Or you can listen on port 5432::

   unet-shell-> nc -ul 5432

Can I use this on a public network?
-----------------------------------
Of course, you can. Just change your network configuration from within
your Vagrantfile.

Features
--------
- Doesn't do any dynamic memory allocation. It features a very simple
  memory allocator. This makes it a suitable candidate for resource
  constrained embedded platforms.
- Liberally licensed under BSD 3-clause License.

Copyright (c) 2017, Cihangir Akturk
