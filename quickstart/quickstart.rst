SOS Quick Start
=================

Introduction
*****************
SOS (pronuounced "sôs"), standing for Scalable Object Store, is a high-performance, indexed, object-oriented database designed to efficiently manage structured data on persistent media.

SOS was created to solve performance and scalability problems found with other time series databases such as InfluxDB, OpenTSDB, and Graphite.

SOS is strictly typed and uses schema to define the objects stored in the database. The schema specifies the attributes that comprise the object and which attributes are indexed.

SOS implements its own back-end storage model. This allows SOS to support:

* Very high insert rates
* Superior query performance
* Flexible storage management

Configuration Options
**********************

* --disable-python
        * The python commands for managing and querying SOS will not be built
* --enable-doc
        * Man pages will be generated for SOS commands and API
* --enable-html
        * HTML documenation will be generated for SOS commands and API

Compile Dependencies
********************

* If --disable-python is not specified
        * Cython >= .29 (Cython 3.0)
        * Python >= 3.6

* If --enable-doc or --enable-html is specified
        * Doxygen

Installation
****************
The following will build SOS and numsos into the directory /home/XXX/BuildSos

cd into the top level sos checkout directory

.. code-block:: console

    ./autogen.sh # this will call autoreconf to generate `configure` script
    mkdir build
    cd build
    ../configure --prefix=/home/XXX/BuildSos [--disable-python] [--enable-debug] \
        [--enable-doc] [--enable-html]
    # add 'PYTHON=/PYTHON/EXECUTABLE/PATH' if PYTHON environment variable not set
    # add `--enable-debug` to turn on debugging logic inside the SOS libraries
    # add `--disable-python` to disable the Python commands and interface libraries
    make && make install


The build will result in /home/XXX/BuildSos/lib/python3.X/site-packages with sosdb and numsos modules. The sosdb module includes the DataSet class and also the Array and Sos modules, which are written in C for efficiency. The numsos module includes the DataSource, DataSink, Stack, and Transform classes.

Set the environment variables appropriately using: 

.. code-block:: console

  export PATH=/home/XXX/BuildSos/bin:$PATH
  export PYTHONPATH=/home/XXX/BuildSos/lib/python3.X/site-packages:$PYTHONPATH

Importing a CSV file and using the command line tools
*********************************

.. list-table:: CSV and Formatting Files

    * - File
      - Use With
      - Description
    * - meminfo_qs.schema.json
      - sos-schema --add
      - A schema definition file
    * - meminfo_qs.map.json
      - sos-import-csv --map 	
      - A file that tells the import tool which CSV columns go to which schema attributes
    * - meminfo_qs.csv
      - sos-import-csv --csv 	
      - 1000 lines of CSV meminfo data 

These files can be obtained from a clone of the wiki under the quickstart directory in the top level of the sos repo.

.. code-block:: console

    > more meminfo_qs.schema.json
    {
    "name" : "meminfo_qs",
    "uuid": "33333333-3333-3333-3333-333333333333",
    "attrs" : [
        { "name" : "timestamp", "type" : "uint64" : "char_array",  "index" : {}  },
        { "name" : "component_id",      "type" : "char_array",  "index" : {}  },
        { "name" : "job_id",    "type" : "char_array",  "index" : {}  },
        { "name" : "app_id",    "type" : "uint64" },
        { "name" : "MemTotal",  "type" : "uint64" },
        { "name" : "MemFree",   "type" : "uint64" },
        ...
        { "name" : "DirectMap2M",       "type" : "uint64" },
        { "name" : "DirectMap1G",       "type" : "uint64" },
        { "name" : "time_job_comp", "type" : "join", "join_attrs" : [ "timestamp", "job_id", "component_id"],
        "index" : {} },
        { "name" : "time_comp_job", "type" : "join", "join_attrs" : [ "timestamp", "component_id", "job_id"],
        "index" : {} },
        { "name" : "job_comp_time", "type" : "join", "join_attrs" : [ "job_id", "component_id", "timestamp" ],
           "index" : {} },
        { "name" : "job_time_comp", "type" : "join", "join_attrs" : [ "job_id", "timestamp", "component_id" ],
           "index" : {} },
        { "name" : "comp_time_job", "type" : "join", "join_attrs" : [ "component_id", "timestamp", "job_id"],
        "index" : {} },
        { "name" : "comp_job_time", "type" : "join", "join_attrs" : [ "component_id", "job_id", "timestamp" ],
           "index" : {} }
         ]
     }
     > more meminfo_qs.map.json
     [
        { "target" : "timestamp", "source" : { "column" : 0 } },
        { "target" : "component_id", "source" : { "column" : 1 } },
        { "target" : "job_id", "source" : { "column" : 2 } },
        { "target" : "app_id", "source" : { "column" :  3 } },
        { "target" : "MemTotal", "source" : { "column" : 4 } },
        { "target" : "MemFree", "source" : { "column" : 5 } },
        ...
        { "target" : "DirectMap2M", "source" : { "column" : 49 } },
        { "target" : "DirectMap1G", "source" : { "column" : 50 } }
     ] ]
     >  more meminfo_qs.csv
     1703108908.000677,2448900245962755385,17165443304811230558,0.0,131928928.0...
     1703108908.000705,3501119766665329829,17326355104910386333,0.0,131928928.0...

Creating a SOS container
************************

1. Create a container if you don't already have one:

.. code-block:: console

 > sos-db --path /dir/my-container --create

Adding a schema to a container

2. Add the schema to the container:

.. code-block:: console

 > sos-schema --path /dir/my-container --add meminfo_qs.schema.json

Querying for schema information

3. Query the schema to see what's in it:

a. Using sos-schema:

.. code-block:: console

 > sos-schema --path /dir/my-container --query --verbose
 meminfo_qs
 Id   Type             Indexed      Name                            
 ---- ---------------- ------------ --------------------------------
   0 TIMESTAMP        True         timestamp
   1 UINT64           True         component_id
   2 UINT64           True         job_id
   3 UINT64                        app_id
   4 UINT64                        MemTotal
   5 UINT64                        MemFree
  ...
  49 UINT64                                DirectMap2M
  50 UINT64                                DirectMap1G
  51 JOIN                     True         time_job_comp [timestamp+job_id+component_id]
  52 JOIN                     True         time_comp_job [timestamp+component_id+job_id]
  53 JOIN                     True         job_comp_time [job_id+component_id+timestamp]
  54 JOIN                     True         job_time_comp [job_id+timestamp+component_id]
  55 JOIN                     True         comp_time_job [component_id+timestamp+job_id]
  56 JOIN                     True         comp_job_time [component_id+job_id+timestamp]

b. OR using sos_cmd:

.. code-block:: console

 > sos_cmd -C /dir/my-container -l
 schema :
    name      : meminfo_qs
    schema_sz : 16728
    gen       : 0
    obj_sz    : 142
    uuid      : 33333333-3333-3333-3333-333333333333
    -attribute : timestamp
        type          : TIMESTAMP
        idx           : 0
        indexed       : 1
        offset        : 16
    -attribute : component_id
        type          : CHAR_ARRAY
        idx           : 1
        indexed       : 1
        offset        : 24
    -attribute : job_id
        type          : CHAR_ARRAY
        idx           : 2
        indexed       : 1
        offset        : 32
    ...
    -attribute : DirectMap2M
        type          : UINT16
        idx           : 49
        indexed       : 0
        offset        : 138
    -attribute : DirectMap1G
        type          : UINT16
        idx           : 50
        indexed       : 0
        offset        : 140
    -attribute : time_job_comp
        type          : JOIN
        idx           : 51
        indexed       : 1
        offset        : 142
    -attribute : time_comp_job
        type          : JOIN
        idx           : 52
        indexed       : 1
        offset        : 142
    -attribute : job_comp_time
        type          : JOIN
        idx           : 53
        indexed       : 1
        offset        : 142
    -attribute : job_time_comp
        type          : JOIN
        idx           : 54
        indexed       : 1
        offset        : 142
    -attribute : comp_time_job
        type          : JOIN
        idx           : 55
        indexed       : 1
        offset        : 142
    -attribute : comp_job_time
        type          : JOIN
        idx           : 56
        indexed       : 1
        offset        : 142

Note that there is no data yet in the container (using sos_cmd):

.. code-block:: console

 > sos_cmd -C /dir/my-container -q -S meminfo_qs -X time_job_comp
 ...
 -------------------------------- ------------------  ... -------------------------------- 
 Records 0/0.

Importing CSV data into a container
***********************************

1. Import the CSV data into the container:

.. code-block:: console

 > sos-import-csv --path /dir/my-container --schema meminfo_qs --map meminfo_qs.map.json --csv meminfo_qs.csv
 Importing from CSV file meminfo_qs.csv into /tmp/my-container using map meminfo_qs.map.json
 Created 1000 records


2. You can monitor the progress from another window like this:

.. code-block:: console

 > sos-monitor --path /dir/my-container --schema meminfo_qs

It will take less than a second for 1000 lines, but you can see progress during larger file loads.
Querying data in a container

3. Query for the data in a container:

 a. Query all the data, using comp_time as an index, which will determine the output order
.. code-block:: console

 > sos_cmd -C /dir/my-container -q -S meminfo_qs -X time_job_comp
 ...
 -------------------------------- ------------------  -------------------------------- 
 Records 1000/1000.

b. Query only for certain variables (also using an index):

.. code-block:: console

 > sos_cmd -C /tmp/my-container/ -q -S meminfo_qs -X time_job_comp -f table -V timestamp -V component_id -V Active 
 timestamp                        component_id       Active             
 timestamp                        component_id Active
 -------------------------------- ------------ ------------------
               1703188156.000797 5427           29557660
               1703188156.000846 36            4825132
               1703188156.000873 4830            1784496
               1703188156.001007 5572           27297788
 ...
               1703188161.001589 9710           24505304
 --------------------------------  ------------------
 Records 1000/1000.

c. Querying with a filter:

.. code-block:: console

 > sos_cmd -C /tmp/my-container/ -q -S meminfo_qs -X time_job_comp -f table -V timestamp -V component_id -V Active -F timestamp:gt:1703188160
 timestamp                        component_id       Active             
 -------------------------------- ------------------ ------------------ 
   ...
               1703188161.001580 282            1999556
               1703188161.001588 5651          111678236
               1703188161.001589 9710           24505304
 --------------------------------  ------------------
 Records 248/248.


d. Querying with multiple filters:

.. code-block:: console

 > sos_cmd -C /tmp/my-container/ -q -S meminfo_qs -X time_job_comp -f table -V timestamp -V component_id -V Active -F timestamp:gt:1703188160 -F component_id:gt:9000
 timestamp                        component_id       Active             
 -------------------------------- ------------------ ------------------ 
 ...
               1703188161.001453               9274           26774688
               1703188161.001530               9593            2218724
               1703188161.001558               9097           57602824
               1703188161.001589               9710           24505304
 -------------------------------- ------------------ ------------------
 Records 23/23.


DSOS Quickstart
=================

Introduction
***************

The Distributed Scalable Object Store (DSOS) (pronounced "dee-sôs") is a layer on top of SOS to enable distributed, parallel ingests and queries. DSOS is intended to be used to use SOS databases across multiple devices as a unified database. Users setup a file, referred to as the cluster configuration file in this context, which names all of the nodes where a SOS database is expected. Using python API or the command line interface dsosql, users can query these SOS databases for data in the same schema. DSOS interfaces are installed alongside SOS, starting with SOS v4, with no additional enable arguments required.

Dsosql
********

For demonstration purposes, let's assume we have two nodes, node1 ande node2, with a SOS database at /storage/sos/database. 
Our cluster configuration file, let's call it dsos.conf, would simply be:

.. code-block:: console

  node1
  node2

Dsosql expects the path to this dsos.conf and the database path for correct functionality. These can be entered as options in to dsosql using the -a and -o options, respectively. They can also be entered after dropping into the dsosql shell, like ldmsd_controller, commands to dsosql can be entered after going into a shell or by echo'ing them into the utility. 

.. code-block:: console

  >dsosql -a dsos.conf -o /storage/sos/database
  Attaching to cluster dsos.conf ... OK
  Opening the container /storage/sos/database ... OK
  dsosql: show_part regex .*
  Name                     Description                                   UID      GID Permission
  ------------------------ ---------------------------------------- -------- -------- -------------
  default                  default                                  33       33       -rw-rw---

  #or

  >dsosql
  dsosql: attach path dsos.conf
  Attaching to cluster dsos.conf ... OK
  dsosql: open path /storage/sos/database
  Opening the container /storage/sos/database ... OK
  dsosql: show_part regex .*
  Name                     Description                                   UID      GID Permission
  ------------------------ ---------------------------------------- -------- -------- -------------
  default                  default                                  33       33       -rw-rw---


  >echo "show_part regex .*" | dsosql -a dsos.conf -o /storage/sos/database
  Attaching to cluster dsos.conf ... OK
  Opening the container /storage/sos/database ... OK
  Name                     Description                                   UID      GID Permission
  ------------------------ ---------------------------------------- -------- -------- -------------
  default                  default                                  33       33       -rw-rw---

Commands available in dsosql are attach, create_part, create_schema, help, import, open, select, set, show, show_part, and show_schema. 


Python API
**********

Like dsosql, python expects a dsos.conf path and a database path. A Sos.Session object is initialized, opened, and then a query setup to begin querying data out of the database. The query initialization expects a max rows returned value for the resultant data object, which will be a pandas DataFrame with columns consisting of the metrics queried and the metrics comprising the index queried. The max rows, or query block size, can typically be set at 1024*1024 though changing block sizes will affect performance.

.. code-block:: python

    import pandas as pd
    from sosdb import Sos
    sess = Sos.Session("dsos.conf")
    cont = sess.open("/storage/sos/database")
    query = Sos.SqlQuery(cont,1024*1024)
    query.select('select Active from meminfo') 
    df = query.next()

The query.next() can be run multiple times to get more data matching the query. The next() will return None if no further data matches the query. A function to return all data matching a query can be written as:

.. code-block:: python

     def get_all_data(self, query):
        df = query.next()
        if df is None:
           return None
        res = df.copy(deep=True)
        while df is not None:
            df = query.next()
            res = pd.concat([df, res])
        del df
        return res

To manually add a record to a DSOS database we can use the insert_df function for a sos container object. 
The dataframe inserted must have rows that match the types and length of the schema being inserted into, otherwise an error will be raised. 
The data will be round robin-ed into the SOS containers referenced in the dsos.conf. 

.. code-block:: python
   
    import pandas as pd
    from sosdb import Sos
    sess = Sos.Session("dsos.conf")
    cont = sess.open("/storage/sos/database")
    schema = cont.schema_by_name('meminfo')
    in_df = {DATAFRAME OF RECORD(S) TO BE INSERTED}
    cont.insert_df(schema,in_df)
       
To update a record in a DSOS database, the update needs to be bounded by a transaction begin and end to prevent data corruption.
Create a key to find the record to be updated, change the values desired, and then update the record.

.. code-block:: python
    import pandas as pd
    from sosdb import Sos
    sess = Sos.Session("dsos.conf")
    cont = sess.open("/storage/sos/database")
    schema = cont.schema_by_name('meminfo')
    attr = schema['job_time_comp']
    key = attr.key(JOB,TIME,COMP)
    obj = attr.find(key)
    cont.transaction_begin()
    obj['component_id'] = 13
    cont.obj_update(obj)
    cont.transaction_end()

