

typedef struct ScheduleItem
{
    uint32_t interval;
    uint32_t start;
    void (*functionName)();
    bool active;
} ScheduleItem;

ScheduleItem theSchedule[16];

bool scheduleFDRS(void (*added_func)(), uint32_t added_interval)
{
    ScheduleItem arg = {.interval = added_interval, .start = millis(), .functionName = added_func, .active = true};
    for (int i = 0; i < 16; i++)
    {
        if (!theSchedule[i].active)
        {
            theSchedule[i] = arg;
            return true;
        }
    }
    DBG("Schedule is full!");
    return false;
}
void handle_schedule()
{
    for (int i = 0; i < 16; i++)
    {
        if (theSchedule[i].active && (millis() - theSchedule[i].start > theSchedule[i].interval))
        {
            theSchedule[i].start = millis();
            theSchedule[i].functionName();
        }
    }
}
