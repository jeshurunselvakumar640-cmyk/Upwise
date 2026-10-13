// Upwise Web Client with Firebase Cloud Database Connectivity & Vercel Standalone Support
import { initializeApp } from "https://www.gstatic.com/firebasejs/10.8.0/firebase-app.js";
import { getAnalytics } from "https://www.gstatic.com/firebasejs/10.8.0/firebase-analytics.js";
import { 
  getFirestore, 
  collection, 
  doc, 
  setDoc, 
  getDoc, 
  addDoc, 
  getDocs, 
  query, 
  orderBy, 
  onSnapshot 
} from "https://www.gstatic.com/firebasejs/10.8.0/firebase-firestore.js";

// Firebase Configuration
const firebaseConfig = {
  apiKey: "AIzaSyCJX_-I4P9kgYJ44BUdL8wMtd-XCtJ6p2s",
  authDomain: "upwise-28b14.firebaseapp.com",
  projectId: "upwise-28b14",
  storageBucket: "upwise-28b14.firebasestorage.app",
  messagingSenderId: "148626797897",
  appId: "1:148626797897:web:0b6ba17464c2317443d698",
  measurementId: "G-XK2SW98DK0"
};

// Initialize Firebase
let fbApp = null;
let analytics = null;
let db = null;
let isFirebaseOnline = false;

try {
  fbApp = initializeApp(firebaseConfig);
  try { analytics = getAnalytics(fbApp); } catch (e) { /* analytics optional in web context */ }
  db = getFirestore(fbApp);
  isFirebaseOnline = true;
  console.log("[Firebase] Successfully connected to Firebase & Cloud Firestore:", firebaseConfig.projectId);
} catch (err) {
  console.warn("[Firebase] Initialization failed, operating in local offline fallback mode:", err);
  isFirebaseOnline = false;
}

// Predefined 18 Academic Questions for Standalone Cloud / Vercel Execution
const DEFAULT_QUESTIONS = [
  // C Programming
  {
    id: 101, topic: "C Programming",
    text: "Which format specifier is standard for printing a memory address in C?",
    options: ["%x", "%p", "%u", "%d"],
    correct_index: 1, difficulty: "Easy"
  },
  {
    id: 102, topic: "C Programming",
    text: "What is the return type of the malloc() function in C?",
    options: ["int*", "char*", "void*", "size_t"],
    correct_index: 2, difficulty: "Easy"
  },
  {
    id: 103, topic: "C Programming",
    text: "Which header file is required to use the 'bool' type in C99?",
    options: ["<stdbool.h>", "<boolean.h>", "<stdlib.h>", "<stdio.h>"],
    correct_index: 0, difficulty: "Easy"
  },
  {
    id: 104, topic: "C Programming",
    text: "What is the result of dereferencing a NULL pointer in C?",
    options: ["Returns 0", "Undefined behaviour / Segmentation fault", "Throws an exception", "Allocates memory"],
    correct_index: 1, difficulty: "Medium"
  },
  {
    id: 105, topic: "C Programming",
    text: "Which standard function is used to free heap-allocated memory in C?",
    options: ["delete()", "destroy()", "free()", "dealloc()"],
    correct_index: 2, difficulty: "Easy"
  },
  {
    id: 106, topic: "C Programming",
    text: "What does the 'sizeof' operator evaluate to in C?",
    options: ["size_t", "int", "long long", "unsigned short"],
    correct_index: 0, difficulty: "Medium"
  },

  // Data Structures
  {
    id: 201, topic: "Data Structures",
    text: "Which data structure follows the LIFO (Last In First Out) principle?",
    options: ["Queue", "Stack", "Binary Tree", "Heap"],
    correct_index: 1, difficulty: "Easy"
  },
  {
    id: 202, topic: "Data Structures",
    text: "What is the average time complexity of Quicksort?",
    options: ["O(n)", "O(n^2)", "O(n log n)", "O(log n)"],
    correct_index: 2, difficulty: "Medium"
  },
  {
    id: 203, topic: "Data Structures",
    text: "What is the time complexity to insert a new node at the head of a singly linked list?",
    options: ["O(1)", "O(n)", "O(log n)", "O(n log n)"],
    correct_index: 0, difficulty: "Easy"
  },
  {
    id: 204, topic: "Data Structures",
    text: "Which data structure operates strictly on a FIFO (First In First Out) basis?",
    options: ["Stack", "Priority Queue", "Queue", "Binary Search Tree"],
    correct_index: 2, difficulty: "Easy"
  },
  {
    id: 205, topic: "Data Structures",
    text: "What is the maximum number of children any node can possess in a binary tree?",
    options: ["1", "2", "3", "Unlimited"],
    correct_index: 1, difficulty: "Easy"
  },
  {
    id: 206, topic: "Data Structures",
    text: "Which searching algorithm requires the input array to be pre-sorted?",
    options: ["Linear Search", "Binary Search", "Hash Lookup", "Breadth First Search"],
    correct_index: 1, difficulty: "Medium"
  },

  // Computer Fundamentals
  {
    id: 301, topic: "Computer Fundamentals",
    text: "How many bits comprise one standard byte in modern computing?",
    options: ["4 bits", "8 bits", "16 bits", "32 bits"],
    correct_index: 1, difficulty: "Easy"
  },
  {
    id: 302, topic: "Computer Fundamentals",
    text: "Which hardware component is primarily known as the brain of the computer?",
    options: ["RAM", "GPU", "CPU", "SSD"],
    correct_index: 2, difficulty: "Easy"
  },
  {
    id: 303, topic: "Computer Fundamentals",
    text: "Which memory type is volatile and loses its data when powered off?",
    options: ["ROM", "RAM", "Hard Disk Drive", "Flash EEPROM"],
    correct_index: 1, difficulty: "Easy"
  },
  {
    id: 304, topic: "Computer Fundamentals",
    text: "What is the primary role of the Operating System kernel?",
    options: ["Web browsing", "Managing hardware and system resources", "Compiling code", "Rendering 3D graphics"],
    correct_index: 1, difficulty: "Medium"
  },
  {
    id: 305, topic: "Computer Fundamentals",
    text: "What does the acronym ALU stand for in processor architecture?",
    options: ["Arithmetic Logic Unit", "Advanced Linking Utility", "Application Layer Unit", "Array Line Unit"],
    correct_index: 0, difficulty: "Easy"
  },
  {
    id: 306, topic: "Computer Fundamentals",
    text: "Which base numbering system utilizes digits 0-9 and letters A-F?",
    options: ["Binary (Base 2)", "Octal (Base 8)", "Decimal (Base 10)", "Hexadecimal (Base 16)"],
    correct_index: 3, difficulty: "Easy"
  }
];

let appState = {
  student: null,
  questions: DEFAULT_QUESTIONS,
  activeQuiz: null,
  activeQuizAnswers: [],
  undoStack: [],
  currentQIndex: 0,
  timerInterval: null,
  secondsLeft: 180,
  lastAttempt: null,
  reviewIndex: 0
};

// Update Cloud Sync Status in UI
function updateCloudStatus(connected, message) {
  const badge = document.getElementById('firebase-status-badge');
  const text = document.getElementById('firebase-status-text');
  if (!badge || !text) return;

  if (connected) {
    badge.classList.add('connected');
    text.textContent = message || 'Firebase Cloud Sync';
  } else {
    badge.classList.remove('connected');
    text.textContent = message || 'Local Engine';
  }
}

// Initialize on DOM load
document.addEventListener('DOMContentLoaded', async () => {
  setupNavigation();
  setupEventListeners();
  updateCloudStatus(isFirebaseOnline, isFirebaseOnline ? 'Firebase Connected' : 'Local Offline Mode');

  await checkProfile();
  if (isFirebaseOnline && db) {
    listenToCloudLeaderboard();
  }
});

async function checkProfile() {
  // 1. Try Local C Server API
  try {
    const res = await fetch('/api/profile');
    if (res.ok) {
      const data = await res.json();
      if (data && data.is_registered) {
        appState.student = data;
        localStorage.setItem('upwise_student', JSON.stringify(data));
        showApp();
        syncStudentToFirestore(data);
        await refreshData();
        return;
      }
    }
  } catch (e) {
    // Expected on Vercel static deployment
  }

  // 2. Check LocalStorage fallback (for Vercel deployment)
  const saved = localStorage.getItem('upwise_student');
  if (saved) {
    try {
      const parsed = JSON.parse(saved);
      if (parsed && parsed.student_id) {
        appState.student = parsed;
        showApp();
        syncStudentToFirestore(parsed);
        await refreshData();
        return;
      }
    } catch (e) {}
  }

  // Otherwise show onboarding form
  document.getElementById('onboarding-view').classList.remove('hidden');
}

async function syncStudentToFirestore(student) {
  if (isFirebaseOnline && db && student) {
    try {
      const studentDocRef = doc(db, "students", student.student_id);
      await setDoc(studentDocRef, {
        name: student.name,
        student_id: student.student_id,
        last_seen: new Date().toISOString()
      }, { merge: true });
      updateCloudStatus(true, 'Firebase Synced');
    } catch (e) {
      console.warn("[Firebase] Student cloud sync notice:", e);
    }
  }
}

function showApp() {
  document.getElementById('onboarding-view').classList.add('hidden');
  document.getElementById('app-layout').classList.remove('hidden');
  document.getElementById('sidebar-name').textContent = appState.student.name;
  document.getElementById('sidebar-id').textContent = appState.student.student_id;
  document.getElementById('user-avatar').textContent = appState.student.name.charAt(0).toUpperCase();
  document.getElementById('welcome-greeting').textContent = `Welcome back, ${appState.student.name}!`;
}

function setupNavigation() {
  document.querySelectorAll('.nav-item').forEach(btn => {
    btn.addEventListener('click', () => {
      const target = btn.getAttribute('data-target');
      if (target === 'topics-anchor') {
        switchView('dashboard-view');
        document.getElementById('topics-anchor').scrollIntoView({ behavior: 'smooth' });
        return;
      }
      document.querySelectorAll('.nav-item').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      switchView(target);
    });
  });
}

function switchView(viewId) {
  document.querySelectorAll('.content-section').forEach(sec => sec.classList.add('hidden'));
  const target = document.getElementById(viewId);
  if (target) target.classList.remove('hidden');
  window.scrollTo({ top: 0, behavior: 'smooth' });
}

// Progress and Analytics Calculation (used for both C server and Vercel cloud)
async function refreshData() {
  // 1. Try local C backend first
  try {
    const [progRes, lbRes] = await Promise.all([
      fetch('/api/progress'),
      fetch('/api/leaderboard')
    ]);
    if (progRes.ok && lbRes.ok) {
      const progress = await progRes.json();
      const leaderboard = await lbRes.json();
      updateDashboard(progress);
      updateProgressView(progress);
      updateLeaderboard(leaderboard);
      return;
    }
  } catch (e) {
    // Fallback to client-side + Firebase analytics for Vercel
  }

  // 2. Client-side & Firebase calculations
  calculateClientProgress();
}

function calculateClientProgress() {
  const attempts = JSON.parse(localStorage.getItem('upwise_attempts') || '[]');
  const myAttempts = appState.student 
    ? attempts.filter(a => a.student_id === appState.student.student_id)
    : attempts;

  const topicsList = ["C Programming", "Data Structures", "Computer Fundamentals"];
  const topicStats = topicsList.map(tName => {
    const forTopic = myAttempts.filter(a => a.topic_name === tName);
    const count = forTopic.length;
    let avg = 0, best = 0, latest = 0;
    if (count > 0) {
      const sum = forTopic.reduce((acc, curr) => acc + (curr.score_percentage || 0), 0);
      avg = Math.round(sum / count);
      best = Math.max(...forTopic.map(a => a.score_percentage || 0));
      latest = forTopic[forTopic.length - 1].score_percentage || 0;
    }
    let cat = "Not attempted";
    if (count > 0) {
      cat = (avg >= 80) ? "Proficient" : (avg >= 50) ? "Developing" : "Needs Improvement";
    }
    return {
      topic_name: tName,
      attempts_count: count,
      average_percentage: avg,
      best_percentage: best,
      latest_percentage: latest,
      category_label: cat,
      has_attempts: count > 0
    };
  });

  const totalQuizzes = myAttempts.length;
  let overallAvg = 0, bestScore = 0;
  if (totalQuizzes > 0) {
    overallAvg = Math.round(myAttempts.reduce((acc, a) => acc + (a.score_percentage || 0), 0) / totalQuizzes);
    bestScore = Math.max(...myAttempts.map(a => a.score_percentage || 0));
  }

  // Adaptive recommendation algorithm
  let unattempted = topicStats.find(t => !t.has_attempts);
  let recTopic = "C Programming";
  let recReason = "Take your first quiz to calibrate personalized recommendations.";

  if (unattempted) {
    recTopic = unattempted.topic_name;
    recReason = "Explore this topic to complete your diagnostic baseline.";
  } else if (totalQuizzes > 0) {
    const sorted = [...topicStats].sort((a, b) => a.average_percentage - b.average_percentage);
    const weakest = sorted[0];
    recTopic = weakest.topic_name;
    if (weakest.average_percentage < 50) {
      recReason = `Current average is ${weakest.average_percentage}%. Prioritize practice here to strengthen core foundations.`;
    } else if (weakest.average_percentage < 80) {
      recReason = `Current average is ${weakest.average_percentage}%. Continued practice will lift you into the Proficient band.`;
    } else {
      recReason = `Proficient across all modules! Revise ${recTopic} to target 100% mastery.`;
    }
  }

  const prog = {
    total_quizzes: totalQuizzes,
    overall_average_percentage: overallAvg,
    highest_score_percentage: bestScore,
    overall_accuracy_percentage: overallAvg,
    adaptive_recommended_topic: recTopic,
    adaptive_recommendation_reason: recReason,
    topics: topicStats,
    recent_attempts: myAttempts.slice(-5).reverse()
  };

  updateDashboard(prog);
  updateProgressView(prog);
}

function updateDashboard(prog) {
  document.getElementById('stat-quizzes').textContent = prog.total_quizzes;
  document.getElementById('stat-avg').textContent = `${prog.overall_average_percentage}%`;
  document.getElementById('stat-best').textContent = `${prog.highest_score_percentage}%`;
  document.getElementById('stat-accuracy').textContent = `${prog.overall_accuracy_percentage}%`;

  if (prog.adaptive_recommended_topic) {
    document.getElementById('adaptive-topic-label').textContent = `Recommended Next: ${prog.adaptive_recommended_topic}`;
    document.getElementById('adaptive-reason-label').textContent = prog.adaptive_recommendation_reason;
    document.getElementById('btn-adaptive-start').onclick = () => startQuiz(prog.adaptive_recommended_topic);
  }

  // Update Topic Modules
  if (prog.topics) {
    prog.topics.forEach(t => {
      let el = null;
      if (t.topic_name.includes('C Program')) el = document.getElementById('perf-c');
      else if (t.topic_name.includes('Data')) el = document.getElementById('perf-ds');
      else el = document.getElementById('perf-cf');

      if (el) {
        if (t.has_attempts) {
          el.textContent = `Best: ${t.best_percentage}% • ${t.attempts_count} attempts`;
          el.style.color = '#34D399';
        } else {
          el.textContent = 'Status: Not attempted yet';
          el.style.color = '#64748B';
        }
      }
    });
  }

  // Recent activity
  const recentCont = document.getElementById('recent-activity-container');
  if (prog.recent_attempts && prog.recent_attempts.length > 0) {
    recentCont.innerHTML = prog.recent_attempts.map(a => `
      <div class="activity-row">
        <div>
          <strong>${a.topic_name}</strong>
          <div style="font-size:0.75rem; color:#94A3B8;">${a.timestamp}</div>
        </div>
        <div style="font-size:0.9rem; font-weight:600;">Score: ${a.score_percentage}%</div>
        <span class="badge ${a.score_percentage >= 80 ? 'badge-success' : a.score_percentage >= 50 ? 'badge-warning' : 'badge-error'}">
          ${a.score_percentage >= 80 ? 'Proficient' : a.score_percentage >= 50 ? 'Developing' : 'Needs Improvement'}
        </span>
      </div>
    `).join('');
  }
}

function updateProgressView(prog) {
  if (!prog.topics) return;
  prog.topics.forEach(t => {
    let prefix = t.topic_name.includes('C Program') ? 'c' : t.topic_name.includes('Data') ? 'ds' : 'cf';
    document.getElementById(`prog-badge-${prefix}`).textContent = t.category_label;
    document.getElementById(`prog-badge-${prefix}`).className = `badge ${t.average_percentage >= 80 ? 'badge-success' : t.average_percentage >= 50 ? 'badge-warning' : 'badge-error'}`;
    document.getElementById(`prog-bar-${prefix}`).style.width = `${t.average_percentage}%`;
    document.getElementById(`prog-info-${prefix}`).textContent = `Attempts: ${t.attempts_count} • Avg: ${t.average_percentage}% • Best: ${t.best_percentage}%`;
  });

  const attemptedOnly = prog.topics.filter(t => t.has_attempts);
  if (attemptedOnly.length > 0) {
    const sorted = [...attemptedOnly].sort((a, b) => b.average_percentage - a.average_percentage);
    document.getElementById('highlight-strong-topic').textContent = sorted[0].topic_name;
    document.getElementById('highlight-weak-topic').textContent = sorted[sorted.length - 1].topic_name;
  }
}

function updateLeaderboard(entries) {
  const tbody = document.getElementById('leaderboard-body');
  if (!entries || entries.length === 0) {
    tbody.innerHTML = '<tr><td colspan="5" style="text-align:center;">No students recorded yet</td></tr>';
    return;
  }

  tbody.innerHTML = entries.map((e, idx) => `
    <tr class="${e.is_current_student ? 'current-user' : ''}">
      <td class="rank-badge ${e.rank === 1 ? 'rank-gold' : e.rank === 2 ? 'rank-silver' : e.rank === 3 ? 'rank-bronze' : ''}">#${e.rank}</td>
      <td><strong>${e.name}</strong> ${e.is_current_student ? '<span class="badge badge-primary" style="margin-left:6px;">YOU</span>' : ''}</td>
      <td style="color:#94A3B8;">${e.student_id}</td>
      <td><strong>${e.average_percentage}%</strong></td>
      <td>${e.quizzes_completed}</td>
    </tr>
  `).join('');
}

// Real-time Cloud Leaderboard Listener (Firebase Firestore)
function listenToCloudLeaderboard() {
  try {
    const lbCol = collection(db, "leaderboard");
    onSnapshot(lbCol, (snapshot) => {
      if (snapshot.empty) return;
      const cloudEntries = [];
      snapshot.forEach(docSnap => {
        cloudEntries.push(docSnap.data());
      });

      // Sort using Quicksort logic (higher avg first, tie-break by completed quizzes)
      cloudEntries.sort((a, b) => {
        if (b.average_percentage !== a.average_percentage) {
          return b.average_percentage - a.average_percentage;
        }
        return b.quizzes_completed - a.quizzes_completed;
      });

      cloudEntries.forEach((e, idx) => {
        e.rank = idx + 1;
        if (appState.student && e.student_id === appState.student.student_id) {
          e.is_current_student = true;
        }
      });

      console.log(`[Firebase] Live cloud leaderboard updated (${cloudEntries.length} entries)`);
      updateLeaderboard(cloudEntries);
      updateCloudStatus(true, 'Firebase Live');
    }, (error) => {
      console.warn("[Firebase] Realtime leaderboard listener error:", error);
    });
  } catch (err) {
    console.warn("[Firebase] Could not attach leaderboard listener:", err);
  }
}

// Quiz Engine
async function startQuiz(topic) {
  let qList = DEFAULT_QUESTIONS.filter(q => q.topic === topic);

  // If local server is available, try fetching
  try {
    const res = await fetch(`/api/questions?topic=${encodeURIComponent(topic)}`);
    if (res.ok) {
      const serverQuestions = await res.json();
      if (serverQuestions && serverQuestions.length > 0) {
        qList = serverQuestions;
      }
    }
  } catch (e) {
    // Standalone fallback
  }

  appState.activeQuiz = { topic, questions: qList };
  appState.activeQuizAnswers = new Array(qList.length).fill(-1);
  appState.undoStack = [];
  appState.currentQIndex = 0;
  appState.secondsLeft = 180;

  document.getElementById('quiz-topic-title').textContent = topic;
  switchView('quiz-view');
  renderQuizQuestion();
  startTimer();
}

function startTimer() {
  if (appState.timerInterval) clearInterval(appState.timerInterval);
  updateTimerDisplay();
  appState.timerInterval = setInterval(() => {
    appState.secondsLeft--;
    updateTimerDisplay();
    if (appState.secondsLeft <= 0) {
      clearInterval(appState.timerInterval);
      submitQuiz();
    }
  }, 1000);
}

function updateTimerDisplay() {
  const m = Math.floor(appState.secondsLeft / 60);
  const s = appState.secondsLeft % 60;
  document.getElementById('quiz-timer').textContent = `Time: ${String(m).padStart(2, '0')}:${String(s).padStart(2, '0')}`;
}

function renderQuizQuestion() {
  const q = appState.activeQuiz.questions[appState.currentQIndex];
  const total = appState.activeQuiz.questions.length;

  document.getElementById('quiz-q-counter').textContent = `Question ${appState.currentQIndex + 1} of ${total}`;
  document.getElementById('quiz-progress-bar').style.width = `${((appState.currentQIndex + 1) / total) * 100}%`;
  document.getElementById('quiz-diff-badge').textContent = q.difficulty || 'Easy';
  document.getElementById('quiz-q-text').textContent = q.text;

  // Options
  const letters = ['A', 'B', 'C', 'D'];
  const userChoice = appState.activeQuizAnswers[appState.currentQIndex];
  const optCont = document.getElementById('quiz-options-container');

  optCont.innerHTML = q.options.map((opt, i) => `
    <div class="option-box ${userChoice === i ? 'selected' : ''}" onclick="selectAnswer(${i})">
      <div class="option-letter">${letters[i]}</div>
      <div class="option-label">${opt}</div>
    </div>
  `).join('');

  // Dots
  const dotsCont = document.getElementById('quiz-dots-nav');
  dotsCont.innerHTML = appState.activeQuiz.questions.map((_, i) => `
    <button class="dot-btn ${i === appState.currentQIndex ? 'active' : ''} ${appState.activeQuizAnswers[i] !== -1 ? 'answered' : ''}" onclick="jumpToQ(${i})">${i + 1}</button>
  `).join('');

  // Nav buttons
  document.getElementById('btn-quiz-prev').disabled = appState.currentQIndex === 0;
  const isLast = appState.currentQIndex === total - 1;
  document.getElementById('btn-quiz-next').classList.toggle('hidden', isLast);
  document.getElementById('btn-quiz-submit').classList.toggle('hidden', !isLast);
}

window.selectAnswer = (idx) => {
  const prev = appState.activeQuizAnswers[appState.currentQIndex];
  appState.activeQuizAnswers[appState.currentQIndex] = idx;
  appState.undoStack.push({ qIndex: appState.currentQIndex, prevChoice: prev });
  renderQuizQuestion();
};

window.jumpToQ = (idx) => {
  appState.currentQIndex = idx;
  renderQuizQuestion();
};

async function submitQuiz() {
  if (appState.timerInterval) clearInterval(appState.timerInterval);

  const payload = {
    topic: appState.activeQuiz.topic,
    answers: appState.activeQuizAnswers,
    question_ids: appState.activeQuiz.questions.map(q => q.id),
    correct_keys: appState.activeQuiz.questions.map(q => q.correct_index)
  };

  let result = null;

  // 1. Try local C backend
  try {
    const res = await fetch('/api/submit', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });
    if (res.ok) {
      result = await res.json();
    }
  } catch (e) {}

  // 2. Client-side scoring if local backend is offline (e.g. on Vercel)
  if (!result) {
    let correct = 0, incorrect = 0, unanswered = 0;
    for (let i = 0; i < payload.answers.length; i++) {
      if (payload.answers[i] === -1) unanswered++;
      else if (payload.answers[i] === payload.correct_keys[i]) correct++;
      else incorrect++;
    }
    const total = payload.answers.length;
    const pct = total > 0 ? Math.round((correct * 100) / total) : 0;
    const cat = (pct >= 80) ? "Proficient" : (pct >= 50) ? "Developing" : "Needs Improvement";
    const now = new Date();
    const timestamp = `${now.getFullYear()}-${String(now.getMonth()+1).padStart(2,'0')}-${String(now.getDate()).padStart(2,'0')} ${String(now.getHours()).padStart(2,'0')}:${String(now.getMinutes()).padStart(2,'0')}`;

    result = {
      attempt_id: `ATT-${Date.now()}`,
      student_id: appState.student ? appState.student.student_id : "UNKNOWN",
      topic_name: payload.topic,
      total_questions: total,
      correct_answers: correct,
      incorrect_answers: incorrect,
      unanswered: unanswered,
      score_percentage: pct,
      performance_category: cat,
      timestamp: timestamp,
      user_answers: payload.answers,
      correct_answers_key: payload.correct_keys
    };

    // Save to LocalStorage
    const attempts = JSON.parse(localStorage.getItem('upwise_attempts') || '[]');
    attempts.push(result);
    localStorage.setItem('upwise_attempts', JSON.stringify(attempts));
  }

  appState.lastAttempt = result;

  // 3. Sync attempt & leaderboard to Firebase Firestore
  if (isFirebaseOnline && db && appState.student) {
    try {
      await addDoc(collection(db, "quizAttempts"), {
        attempt_id: result.attempt_id,
        student_id: appState.student.student_id,
        student_name: appState.student.name,
        topic_name: result.topic_name,
        total_questions: result.total_questions,
        correct_answers: result.correct_answers,
        incorrect_answers: result.incorrect_answers,
        unanswered: result.unanswered,
        score_percentage: result.score_percentage,
        performance_category: result.performance_category,
        timestamp: result.timestamp,
        user_answers: result.user_answers,
        correct_answers_key: result.correct_answers_key,
        created_at: new Date().toISOString()
      });

      // Update Student's Cloud Leaderboard entry
      const lbDocRef = doc(db, "leaderboard", appState.student.student_id);
      const lbSnap = await getDoc(lbDocRef);
      let completed = 1;
      let avg = result.score_percentage;
      if (lbSnap.exists()) {
        const oldData = lbSnap.data();
        completed = (oldData.quizzes_completed || 0) + 1;
        avg = Math.round(((oldData.average_percentage || 0) * (completed - 1) + result.score_percentage) / completed);
      }

      await setDoc(lbDocRef, {
        student_id: appState.student.student_id,
        name: appState.student.name,
        average_percentage: avg,
        quizzes_completed: completed,
        last_active: new Date().toISOString()
      }, { merge: true });

      console.log("[Firebase] Successfully saved attempt to Cloud Firestore");
      updateCloudStatus(true, 'Attempt Synced to Cloud');
    } catch (fbErr) {
      console.warn("[Firebase] Cloud attempt sync notice:", fbErr);
    }
  }

  showResults(result);
  await refreshData();
}

function showResults(res) {
  document.getElementById('results-meta-subtitle').textContent = `Topic: ${res.topic_name} • ${res.timestamp}`;
  document.getElementById('results-score-pct').textContent = `${res.score_percentage}%`;
  document.getElementById('res-correct').textContent = res.correct_answers;
  document.getElementById('res-incorrect').textContent = res.incorrect_answers;
  document.getElementById('res-unanswered').textContent = res.unanswered;

  const catBadge = document.getElementById('results-category-badge');
  catBadge.textContent = res.performance_category;
  catBadge.className = `badge ${res.score_percentage >= 80 ? 'badge-success' : res.score_percentage >= 50 ? 'badge-warning' : 'badge-error'}`;

  switchView('results-view');
}

// Review Answers
function startReview() {
  appState.reviewIndex = 0;
  switchView('review-view');
  renderReviewQuestion();
}

function renderReviewQuestion() {
  const q = appState.activeQuiz.questions[appState.reviewIndex];
  const userAns = appState.lastAttempt.user_answers[appState.reviewIndex];
  const correctAns = appState.lastAttempt.correct_answers_key[appState.reviewIndex];
  const total = appState.activeQuiz.questions.length;

  document.getElementById('review-meta').textContent = `Question ${appState.reviewIndex + 1} of ${total}`;
  document.getElementById('review-q-text').textContent = q.text;

  const badge = document.getElementById('review-badge-status');
  if (userAns === -1) {
    badge.textContent = 'UNANSWERED';
    badge.className = 'badge badge-warning';
  } else if (userAns === correctAns) {
    badge.textContent = 'CORRECT (+1)';
    badge.className = 'badge badge-success';
  } else {
    badge.textContent = 'INCORRECT';
    badge.className = 'badge badge-error';
  }

  const letters = ['A', 'B', 'C', 'D'];
  const optCont = document.getElementById('review-options-container');

  optCont.innerHTML = q.options.map((opt, i) => {
    let cls = '';
    let tag = '';
    if (i === correctAns) {
      cls = 'selected';
      tag = '<span class="badge badge-success" style="margin-left:auto;">Correct Key</span>';
    } else if (i === userAns) {
      cls = 'error-selected';
      tag = '<span class="badge badge-error" style="margin-left:auto;">Your Choice</span>';
    }
    return `
      <div class="option-box ${cls}" style="cursor:default;">
        <div class="option-letter">${letters[i]}</div>
        <div class="option-label">${opt}</div>
        ${tag}
      </div>
    `;
  }).join('');

  document.getElementById('btn-review-prev').disabled = appState.reviewIndex === 0;
  document.getElementById('btn-review-next').disabled = appState.reviewIndex === total - 1;
}

function setupEventListeners() {
  // Onboarding
  document.getElementById('onboarding-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const name = document.getElementById('student-name').value.trim();
    const id = document.getElementById('student-id').value.trim();

    // 1. Try saving to local C backend
    try {
      await fetch('/api/profile', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ name, student_id: id })
      });
    } catch (e) {}

    // 2. Save to LocalStorage (works everywhere, including Vercel)
    const student = { name, student_id: id, is_registered: true };
    appState.student = student;
    localStorage.setItem('upwise_student', JSON.stringify(student));

    // 3. Save to Firebase Firestore
    if (isFirebaseOnline && db) {
      try {
        await setDoc(doc(db, "students", id), {
          name: name,
          student_id: id,
          registered_at: new Date().toISOString()
        }, { merge: true });
        updateCloudStatus(true, 'Firebase Connected');
      } catch (fbErr) {
        console.warn("[Firebase] Could not save student to cloud:", fbErr);
      }
    }

    showApp();
    await refreshData();
  });

  // Start Topic Buttons
  document.querySelectorAll('.btn-start-topic').forEach(btn => {
    btn.addEventListener('click', () => {
      const topic = btn.getAttribute('data-topic');
      startQuiz(topic);
    });
  });

  // Quiz Navigation
  document.getElementById('btn-quiz-prev').addEventListener('click', () => {
    if (appState.currentQIndex > 0) {
      appState.currentQIndex--;
      renderQuizQuestion();
    }
  });

  document.getElementById('btn-quiz-next').addEventListener('click', () => {
    if (appState.currentQIndex < appState.activeQuiz.questions.length - 1) {
      appState.currentQIndex++;
      renderQuizQuestion();
    }
  });

  document.getElementById('btn-quiz-submit').addEventListener('click', () => {
    if (confirm('Are you sure you want to finish and submit your quiz?')) {
      submitQuiz();
    }
  });

  document.getElementById('btn-quiz-undo').addEventListener('click', () => {
    if (appState.undoStack.length > 0) {
      const last = appState.undoStack.pop();
      appState.activeQuizAnswers[last.qIndex] = last.prevChoice;
      appState.currentQIndex = last.qIndex;
      renderQuizQuestion();
    }
  });

  // Results Actions
  document.getElementById('btn-go-review').addEventListener('click', startReview);
  document.getElementById('btn-retry-quiz').addEventListener('click', () => startQuiz(appState.activeQuiz.topic));
  document.getElementById('btn-back-dash').addEventListener('click', () => {
    switchView('dashboard-view');
    document.querySelectorAll('.nav-item').forEach((b, i) => b.classList.toggle('active', i === 0));
  });

  // Review Actions
  document.getElementById('btn-review-back-results').addEventListener('click', () => switchView('results-view'));
  document.getElementById('btn-review-prev').addEventListener('click', () => {
    if (appState.reviewIndex > 0) {
      appState.reviewIndex--;
      renderReviewQuestion();
    }
  });
  document.getElementById('btn-review-next').addEventListener('click', () => {
    if (appState.reviewIndex < appState.activeQuiz.questions.length - 1) {
      appState.reviewIndex++;
      renderReviewQuestion();
    }
  });

  // Settings
  document.getElementById('settings-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const name = document.getElementById('edit-name').value.trim();
    const id = document.getElementById('edit-id').value.trim();

    try {
      await fetch('/api/profile', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ name, student_id: id })
      });
    } catch (e) {}

    const student = { name, student_id: id, is_registered: true };
    appState.student = student;
    localStorage.setItem('upwise_student', JSON.stringify(student));

    if (isFirebaseOnline && db) {
      try {
        await setDoc(doc(db, "students", id), {
          name: name,
          student_id: id,
          updated_at: new Date().toISOString()
        }, { merge: true });
      } catch (e) {
        console.warn(e);
      }
    }

    showApp();
    await refreshData();
    alert('Profile updated successfully!');
  });
}
