program BAKERY_PROG {
    version BAKERY_VER {
        int GET_TICKET_NUMBER(int) = 1;
        hyper GET_SERVICE_TIME(int) = 2;
    } = 1;
} = 0x30000001;
