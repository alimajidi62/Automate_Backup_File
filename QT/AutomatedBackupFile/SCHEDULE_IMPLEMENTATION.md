# Schedule Tab Implementation

## Overview
This document describes the implementation of the Schedule Tab feature, which allows users to create and manage automated backup schedules.

## Components

### 1. BackupSchedule (Model)
**Files**: `backupschedule.h`, `backupschedule.cpp`

**Purpose**: Represents a single backup schedule with all its properties.

**Key Features**:
- **Schedule Frequencies**:
  - `Daily`: Runs once per day at specified time
  - `Weekly`: Runs on selected weekdays at specified time
  - `Monthly`: Runs on a specific day of the month at specified time
  - `Custom`: Runs at custom intervals (in minutes)

- **Properties**:
  - Unique ID (UUID)
  - Schedule name
  - Frequency type
  - Time of day
  - Enabled/disabled state
  - Last run timestamp
  - Next run timestamp
  - Weekly: List of days (Monday=1 to Sunday=7)
  - Monthly: Day of month (1-31)
  - Custom: Interval in minutes

- **Methods**:
  - `calculateNextRun()`: Calculates when the schedule should next run based on frequency
  - `shouldRunNow()`: Checks if the schedule is due to run (within 1-minute tolerance)
  - `toJson()`/`fromJson()`: JSON serialization for persistence

### 2. ScheduleManager (Controller)
**Files**: `schedulemanager.h`, `schedulemanager.cpp`

**Purpose**: Manages the collection of schedules and timer-based execution.

**Key Features**:
- **Schedule Management**:
  - Add/remove/get schedules
  - Validate schedules (name not empty, valid time, weekly has days, etc.)
  - Duplicate name prevention

- **Timer-Based Execution**:
  - QTimer that checks schedules at regular intervals (default: 60 seconds)
  - Configurable check interval
  - Enable/disable scheduler globally

- **Persistence**:
  - Save schedules to `schedules.json`
  - Load schedules from file
  - Auto-save on add/edit/remove operations

- **Signals**:
  - `scheduleAdded`: Emitted when a schedule is added
  - `scheduleRemoved`: Emitted when a schedule is removed
  - `scheduleUpdated`: Emitted when a schedule is modified
  - `scheduleTriggered`: Emitted when a schedule should run (includes scheduleId and scheduleName)
  - `schedulerStateChanged`: Emitted when scheduler is enabled/disabled

### 3. ScheduleTab (View)
**Files**: `scheduletab.h`, `scheduletab.cpp`, `scheduletab.ui`

**Purpose**: User interface for managing schedules.

**UI Components**:
- **Schedules Table**: Displays all schedules with columns:
  - Schedule Name
  - Frequency (Daily/Weekly/Monthly/Custom)
  - Time (HH:mm format)
  - Next Run (YYYY-MM-DD HH:mm)
  - Enabled (Yes/No)

- **Buttons**:
  - Add Schedule: Opens dialog for creating new schedule
  - Edit: Opens dialog for editing selected schedule
  - Remove: Deletes selected schedule with confirmation

- **Enable Scheduler Checkbox**: Master switch to enable/disable all automatic backups

- **Quick Schedule Form**:
  - Schedule Name input
  - Frequency dropdown
  - Time picker
  - Add Quick Schedule button (creates schedule without opening dialog)

- **Status Label**: Shows "Scheduler is running" or "Scheduler is stopped"

**Schedule Dialog**:
- Name field
- Frequency dropdown
- Time picker
- Weekdays list (multi-select, for Weekly schedules)
- Day of Month spinner (1-31, for Monthly schedules)
- Interval spinner (1-1440 minutes, for Custom schedules)
- Enabled checkbox
- OK/Cancel buttons

**Features**:
- Loads saved schedules on startup
- Auto-saves schedules on any change
- Restores scheduler state (enabled/disabled) from saved data
- Disables Edit/Remove buttons when no selection
- Validates schedule name before adding

### 4. MainWindow Integration
**Files**: `mainwindow.h`, `mainwindow.cpp`

**Integration Points**:
- Gets ScheduleManager reference from ScheduleTab
- Connects to `scheduleTriggered` signal
- When schedule triggers:
  1. Marks schedule as run (updates lastRun and nextRun)
  2. Calls `onStartBackup()` to execute backup automatically
  3. Updates status bar with scheduled backup notification

## Usage Flow

### Creating a Schedule
1. User clicks "Add Schedule" button or uses Quick Schedule form
2. Fills in schedule details (name, frequency, time, etc.)
3. ScheduleTab creates `BackupSchedule` object
4. ScheduleManager validates and adds schedule
5. Schedule is saved to `schedules.json`
6. Table refreshes to show new schedule

### Editing a Schedule
1. User selects schedule in table
2. Clicks "Edit" button
3. Dialog opens with current schedule properties
4. User modifies properties
5. Changes are applied to existing schedule object
6. Next run time is recalculated
7. Changes saved to `schedules.json`
8. Table updates to reflect changes

### Automatic Execution
1. User enables scheduler checkbox
2. ScheduleManager starts QTimer (60-second intervals)
3. Timer fires, calls `checkSchedules()`
4. For each enabled schedule:
   - Calls `shouldRunNow()` to check if due
   - If due, emits `scheduleTriggered` signal
   - MainWindow receives signal
   - Marks schedule as run
   - Triggers backup automatically
5. Schedule's lastRun and nextRun are updated

### Schedule Frequency Logic

**Daily**:
```cpp
QDateTime nextRun = QDateTime::currentDateTime();
nextRun.setTime(scheduleTime);
if (nextRun <= currentTime) {
    nextRun = nextRun.addDays(1);  // Already passed today, schedule for tomorrow
}
```

**Weekly**:
```cpp
// Find next occurrence of any selected weekday
for (int daysAhead = 0; daysAhead <= 7; ++daysAhead) {
    QDateTime candidate = currentDateTime.addDays(daysAhead);
    if (weekDays.contains(candidate.date().dayOfWeek()) && candidate.time() >= scheduleTime) {
        return candidate;
    }
}
```

**Monthly**:
```cpp
QDateTime nextRun = QDateTime::currentDateTime();
nextRun.setTime(scheduleTime);

// Set to specified day of month
QDate targetDate(nextRun.date().year(), nextRun.date().month(), dayOfMonth);

// Validate day exists in month (e.g., Feb 30 -> Feb 28)
if (!targetDate.isValid()) {
    targetDate.setDate(nextRun.date().year(), nextRun.date().month(), 
                       targetDate.daysInMonth());
}

if (targetDate < currentDate || (targetDate == currentDate && scheduleTime <= currentTime)) {
    // Move to next month
    targetDate = targetDate.addMonths(1);
}
```

**Custom**:
```cpp
// Simply add interval minutes to last run time
// Or to current time if never run
QDateTime nextRun = lastRun.isValid() ? lastRun : QDateTime::currentDateTime();
nextRun = nextRun.addSecs(intervalMinutes * 60);
```

## Data Persistence

**File**: `schedules.json` (in application directory)

**Format**:
```json
{
    "schedules": [
        {
            "id": "{uuid}",
            "name": "Daily Backup",
            "frequency": 0,
            "time": "14:00:00",
            "enabled": true,
            "lastRun": "2024-01-15T14:00:00",
            "nextRun": "2024-01-16T14:00:00",
            "weekDays": [],
            "dayOfMonth": 1,
            "intervalMinutes": 60
        }
    ],
    "schedulerEnabled": true,
    "checkIntervalSeconds": 60
}
```

## Configuration

**Check Interval**: Default is 60 seconds. Can be changed via:
```cpp
scheduleManager->setCheckInterval(30);  // Check every 30 seconds
```

**Tolerance Window**: Schedule is considered "due" if within 1 minute of scheduled time. This prevents missing schedules if check happens slightly after scheduled time.

## Error Handling

- Invalid schedules are rejected (empty name, no weekdays for weekly, invalid day for monthly)
- Duplicate schedule names are prevented
- File I/O errors are logged (save/load failures)
- Invalid JSON data is handled gracefully

## Dependencies

- Qt Widgets (QDialog, QTableWidget, QFormLayout, etc.)
- Qt Core (QTimer, QDateTime, QJsonDocument)
- BackupEngine (for executing backups)
- SourceManager and DestinationManager (for backup source/destination pairs)

## Future Enhancements

Possible improvements:
- Schedule source/destination selection (currently uses all enabled sources)
- Missed schedule handling (run immediately if missed)
- Schedule history/logging
- Multiple time slots per day for Daily schedules
- Exclusion dates/times (holidays, maintenance windows)
- Email notifications when scheduled backups complete
- Retry logic for failed scheduled backups
