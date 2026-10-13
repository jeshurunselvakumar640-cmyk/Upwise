# UPWISE | Learn. Practise. Progress.

**Upwise** is a desktop-based E-Learning Quiz and Progress Tracking System built entirely in standard **C (C99)** and **raylib**. It features a modern dark navy design system, an interactive multiple-choice quiz engine, historical attempt analytics, adaptive practice recommendations, and a local student cohort leaderboard.

---

## 1. Key Features & Screens

1. **Student Profile Onboarding (`SCREEN_PROFILE`)**
   - Welcomes students with custom geometric vector branding.
   - Collects and validates Student Name and Student ID (prevents empty inputs, whitespace-only names, buffer overflows).
   - Automatically saves and loads credentials to/from local storage (`data/students.txt`).
   - Profile settings screen (`SCREEN_SETTINGS`) enables updating identity at any time.

2. **Dashboard (`SCREEN_DASHBOARD`)**
   - Personalized welcome banner: *"Welcome back, [Name]! Small steps today. Greater progress tomorrow."*
   - Four real-time statistical cards: **Quizzes Completed**, **Average Score %**, **Best Score %**, and **Overall Accuracy %**.
   - **Adaptive Practice Recommendation Banner**: Rule-based diagnostic algorithm identifying weak or unattempted subjects.
   - Module cards for 3 academic topics with previous performance summaries and direct quiz launch.
   - Chronological recent quiz activity log.

3. **Multiple-Choice Quiz Engine (`SCREEN_QUIZ`)**
   - Question sequence managed via a **Queue (FIFO)** data structure.
   - Answer changes recorded onto a **Stack (LIFO)** data structure, enabling a dedicated **"Undo Last Answer"** button.
   - Countdown timer with automatic submission on expiry.
   - Visual progress bar and quick question navigation dots (color-coded for answered/unanswered).
   - Confirmation modal prevents accidental early submissions.

4. **Results & Answer Review (`SCREEN_RESULTS` & `SCREEN_REVIEW`)**
   - Categorizes performance into:
     - **Proficient** (80% – 100%)
     - **Developing** (50% – 79%)
     - **Needs Improvement** (< 50%)
   - Breakdown of correct, incorrect, and unanswered questions.
   - Detailed **Answer Review** screen stepping through each question with green/red answer highlighting and answer keys.
   - Persists each completed attempt exactly once to local storage (`data/attempts.txt`).

5. **Learning Analytics & Progress (`SCREEN_PROGRESS`)**
   - Topic-wise mastery cards with progress bars and individual attempt counts.
   - Identifies **Strongest Topic** and **Weakest Topic** based on empirical averages.
   - **Custom Raylib Bar Chart** rendering the score percentage trend across the last 7 attempts with hover tooltips and gridlines.

6. **Cohort Leaderboard (`SCREEN_LEADERBOARD`)**
   - Ranks local peer students and the active user using **Quicksort in C**.
   - Dual-criterion ranking: Primary by highest average score percentage; secondary tie-breaker by quizzes completed.
   - **Search by Student ID** algorithm for instant lookup in the cohort table.

---

## 2. Data Structures & Algorithms Implemented

| Data Structure / Algorithm | Source File | Purpose & Usage in Upwise |
|---|---|---|
| **Queue (FIFO)** | [`src/data_structures.c`](file:///c:/Codes/sem%203/Upwise/src/data_structures.c) | Manages the question sequence during active quiz sessions (`queue_enqueue`, `queue_dequeue`). |
| **Stack (LIFO)** | [`src/data_structures.c`](file:///c:/Codes/sem%203/Upwise/src/data_structures.c) | Tracks user answer modifications for the **Undo Last Answer** feature (`stack_push`, `stack_pop`). |
| **Singly Linked List** | [`src/data_structures.c`](file:///c:/Codes/sem%203/Upwise/src/data_structures.c) | Dynamically manages chronological quiz attempt history (`attempt_list_add`, traversal, lookup, memory release). |
| **Quicksort (O(n log n))** | [`src/leaderboard.c`](file:///c:/Codes/sem%203/Upwise/src/leaderboard.c) | Sorts leaderboard entries descending by average score, with tie-breaking by quizzes completed. |
| **Searching Algorithm** | [`src/leaderboard.c`](file:///c:/Codes/sem%203/Upwise/src/leaderboard.c) | Searches cohort student records by exact student ID. |
| **Adaptive Rule Engine** | [`src/progress.c`](file:///c:/Codes/sem%203/Upwise/src/progress.c) | Analyzes topic averages from the attempt linked list to recommend focus subjects (<50% urgent, 50-79% developing). |

---

## 3. Project File Structure

```
Upwise/
├── src/
│   ├── main.c              # Application entry point, screens, and main loop
│   ├── app.h               # Central application state & screen enum
│   ├── ui.h / ui.c         # Modern dark theme design system & reusable widgets
│   ├── quiz.h / quiz.c     # Question bank, quiz session engine & question sequence
│   ├── student.h / student.c # Profile validation and storage
│   ├── progress.h / progress.c # Analytics calculation & adaptive recommendation
│   ├── leaderboard.h / leaderboard.c # Quicksort & student ID search
│   ├── data_structures.h / data_structures.c # Queue, Stack, and Linked List implementations
│   └── compat.h / compat.c # MinGW 32-bit runtime compatibility shims
├── data/
│   ├── questions.txt       # Predefined question bank (auto-generated if missing)
│   ├── students.txt        # Local student profile storage
│   ├── attempts.txt        # Detailed historical quiz attempt records
│   └── leaderboard.txt     # Cohort leaderboard records
├── tests/
│   ├── test_ds.c           # Unit tests for Queue, Stack, Linked List, Quicksort, Search
│   └── test_persistence.c  # Integration tests for file handling, analytics, and leaderboard
├── raylib/
│   ├── include/            # raylib.h, raymath.h, rlgl.h
│   └── lib/                # libraylib.a (Windows MinGW)
├── build.bat               # Windows build automation script
└── README.md               # Complete documentation
```

---

## 4. Visual Design System

- **Background Main**: `#0B1020` (Dark Navy)
- **Secondary Background**: `#111A2E` (Sidebar and panels)
- **Card Background**: `#172238` with subtle border `#253556`
- **Primary Accent**: `#5B8CFF` (Electric Blue)
- **Secondary Accent**: `#8B5CF6` (Vibrant Violet)
- **Success / Proficient**: `#34D399` (Emerald Green)
- **Warning / Developing**: `#FBBF24` (Amber Gold)
- **Error / Needs Improvement**: `#FB7185` (Coral Red)

---

## 5. Building & Running

### Requirements
- **OS**: Windows (10 / 11)
- **Compiler**: GCC (MinGW / MinGW-w64) installed and in your `PATH`.
- Raylib header and library files are pre-configured locally inside `raylib/include` and `raylib/lib`.

### Command Line / PowerShell

1. **Build the Desktop Application & Tests**:
   ```cmd
   build.bat
   ```
   *Compiles `Upwise.exe` with High-DPI anti-aliased TrueType font engine and tests.*

2. **Run Desktop Application**:
   ```cmd
   build.bat run
   ```
   *Or run `.\Upwise.exe` directly.*

3. **Run All Automated Tests**:
   ```cmd
   build.bat test
   ```
   *Executes all 30 data structure unit tests and persistence integration tests.*

4. **Clean Binaries**:
   ```cmd
   build.bat clean
   ```

---

## 6. Viva & Academic Defense Cheatsheet

1. **Why use a Queue for the Quiz?**
   - Guarantees questions are visited in an organized sequential FIFO order. Enqueue appends questions at the rear; dequeue advances cleanly without memory corruption.
2. **Why use a Stack for Undo?**
   - Answers follow a LIFO (Last-In, First-Out) history. When a user changes an answer, the previous selection is pushed onto the stack. Clicking "Undo" pops the top action and restores the prior selection.
3. **Why use a Linked List for Attempts?**
   - The number of quizzes taken across sessions is unknown and dynamic. A linked list grows dynamically in O(1) time without requiring costly reallocations of contiguous memory.
4. **How does Quicksort rank students?**
   - It partitions entries around a pivot using the composite comparator: primary key `average_percentage` descending, and tie-breaker `quizzes_completed` descending.
