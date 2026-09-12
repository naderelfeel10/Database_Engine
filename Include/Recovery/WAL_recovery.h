#ifndef WAL_RECOVERY_H
#define WAL_RECOVERY_H

#include "Recovery\WAL_record.h"

#include <fcntl.h>
#include <io.h>
#include <vector>
#include <memory>
#include <stdexcept>
#include <share.h>
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