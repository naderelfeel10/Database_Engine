#ifndef WAL_RECOVERY_H
#define WAL_RECOVERY_H

#include "Recovery/WAL_record.h"

#include <fcntl.h>
#include <vector>
#include <memory>
#include <stdexcept>
//#include <share.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;

//from row logs, recover
class WALRecovery{

private:
    int fd;

public:

    WALRecovery(const char* path);
    ~WALRecovery();

    vector<WALRecord> read_all_records();
};

#endif