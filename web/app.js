// Upwise Web Client with Firebase Cloud Database Connectivity
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
  try { analytics = getAnalytics(fbApp); } catch (e) { /* analytics optional in local context */ }
  db = getFirestore(fbApp);
  isFirebaseOnline = true;
  console.log("[Firebase] Successfully initialized Firebase & Firestore:", firebaseConfig.projectId);
} catch (err) {
  console.warn("[Firebase] Initialization failed, operating in local offline fallback mode:", err);
  isFirebaseOnline = false;
}

let appState = {
  student: null,
  questions: [],
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
    text.textContent = message || 'Local C Engine';
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
  try {
    const res = await fetch('/api/profile');
    const data = await res.json();
    if (data && data.is_registered) {
      appState.student = data;
      showApp();

      // Check / Sync profile with Firestore
      if (isFirebaseOnline && db) {
        try {
          const studentDocRef = doc(db, "students", data.student_id);
          const snap = await getDoc(studentDocRef);
          if (!snap.exists()) {
            await setDoc(studentDocRef, {
              name: data.name,
              student_id: data.student_id,
              last_seen: new Date().toISOString()
            }, { merge: true });
          }
          updateCloudStatus(true, 'Firebase Synced');
        } catch (e) {
          console.warn("[Firebase] Profile sync check notice:", e);
        }
      }

      await refreshData();
    } else {
      document.getElementById('onboarding-view').classList.remove('hidden');
    }
  } catch (e) {
    document.getElementById('onboarding-view').classList.remove('hidden');
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

async function refreshData() {
  try {
    const [progRes, lbRes] = await Promise.all([
      fetch('/api/progress'),
      fetch('/api/leaderboard')
    ]);
    const progress = await progRes.json();
    const leaderboard = await lbRes.json();

    updateDashboard(progress);
    updateProgressView(progress);
    updateLeaderboard(leaderboard);
  } catch (e) {
    console.error('Error refreshing local data:', e);
  }
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

  if (prog.strongest_topic) document.getElementById('highlight-strong-topic').textContent = prog.strongest_topic;
  if (prog.weakest_topic) document.getElementById('highlight-weak-topic').textContent = prog.weakest_topic;
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

      // Sort using same ranking rules (higher avg first, tie-break by completed quizzes)
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

      console.log(`[Firebase] Live cloud leaderboard synced (${cloudEntries.length} entries)`);
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
  try {
    const res = await fetch(`/api/questions?topic=${encodeURIComponent(topic)}`);
    const qList = await res.json();
    if (!qList || qList.length === 0) {
      alert('No questions found for this topic.');
      return;
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
  } catch (e) {
    console.error('Error starting quiz:', e);
  }
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

  try {
    // 1. Submit to local C backend
    const res = await fetch('/api/submit', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });
    const result = await res.json();
    appState.lastAttempt = result;

    // 2. Sync attempt & leaderboard to Firebase Firestore
    if (isFirebaseOnline && db && appState.student) {
      try {
        // Save Attempt to Firestore
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

        console.log("[Firebase] Successfully synced attempt to Cloud Firestore");
        updateCloudStatus(true, 'Attempt Synced to Cloud');
      } catch (fbErr) {
        console.warn("[Firebase] Could not push attempt to cloud:", fbErr);
      }
    }

    showResults(result);
    await refreshData();
  } catch (e) {
    console.error('Error submitting quiz:', e);
  }
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

    // 1. Save to local C backend
    const res = await fetch('/api/profile', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ name, student_id: id })
    });
    const data = await res.json();
    if (data.success) {
      appState.student = { name, student_id: id, is_registered: true };

      // 2. Save to Firebase Firestore
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
    }
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
    const res = await fetch('/api/profile', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ name, student_id: id })
    });
    const data = await res.json();
    if (data.success) {
      appState.student = { name, student_id: id, is_registered: true };

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
    }
  });
}
