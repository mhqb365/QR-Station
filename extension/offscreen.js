/**
 * Offscreen script to play notification sounds
 */

chrome.runtime.onMessage.addListener((request, sender, sendResponse) => {
  if (request.action === "play-notification-sound") {
    handleNotification(request.data);
  }
});

function handleNotification(data) {
  // Ưu tiên đọc số tiền nếu có dữ liệu
  if (data) {
    let transactions = [];
    if (data.transactions && Array.isArray(data.transactions)) {
      transactions = data.transactions;
    } else if (data.amount || data.transferAmount) {
      transactions = [data];
    } else if (Array.isArray(data)) {
      transactions = data;
    }

    if (transactions.length > 0) {
      transactions.forEach((trans, index) => {
        setTimeout(() => {
          const amount = trans.transferAmount || trans.amount || 0;
          if (amount > 0) {
            speakAmount(amount);
          } else {
            playPaymentSound(); // Fallback nếu không có số tiền
          }
        }, index * 4000); // Cách nhau 4 giây nếu có nhiều giao dịch
      });
      return;
    }
  }

  // Nếu không có data hoặc không parse được, phát âm thanh mặc định
  playPaymentSound();
}

function speakAmount(amount) {
  const amountStr = amount.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ".");
  const text = `Đã nhận ${amountStr} đồng.`;

  // Phát tiếng Ting Ting từ file notify.mp3 vừa tải về
  const introAudio = new Audio("notify.mp3");
  introAudio
    .play()
    .then(() => {
      console.log("Played custom ting ting sound from notify.mp3");
    })
    .catch((err) => {
      console.log("Custom sound failed, using synthesized sound:", err);
      playSynthesizedSound();
    });

  // Đợi 1.2 giây cho tiếng ting ting phát rồi mới đọc lời nhắn
  setTimeout(() => {
    const googleTtsUrl = `https://translate.google.com/translate_tts?ie=UTF-8&q=${encodeURIComponent(text)}&tl=vi&client=tw-ob`;
    const audio = new Audio(googleTtsUrl);

    audio
      .play()
      .then(() => {
        console.log("Played Google TTS voice");
      })
      .catch((err) => {
        console.log("Google TTS failed, falling back to Web Speech API:", err);
        playWebSpeech(text);
      });
  }, 1200);
}

function playWebSpeech(text) {
  // Đảm bảo voices đã load (đặc biệt quan quan trọng trên Chrome)
  const synth = window.speechSynthesis;

  const speak = () => {
    const utterance = new SpeechSynthesisUtterance(text);
    utterance.lang = "vi-VN";
    utterance.rate = 1.0;
    utterance.pitch = 1.1;

    const voices = synth.getVoices();
    // Ưu tiên các giọng nữ Việt Nam
    const viVoice =
      voices.find(
        (v) =>
          v.lang.startsWith("vi") &&
          (v.name.includes("Google") ||
            v.name.includes("Female") ||
            v.name.includes("Linh") ||
            v.name.includes("An")),
      ) || voices.find((v) => v.lang.startsWith("vi"));

    if (viVoice) {
      utterance.voice = viVoice;
    }

    synth.speak(utterance);
  };

  if (synth.getVoices().length === 0) {
    synth.onvoiceschanged = speak;
  } else {
    speak();
  }
}

function playPaymentSound() {
  const audio = new Audio("notify.mp3");
  audio
    .play()
    .then(() => {
      console.log("Played notification from notify.mp3");
    })
    .catch((err) => {
      console.log("Could not play notify.mp3, using synthesized sound:", err);
      playSynthesizedSound();
    });
}

function playSynthesizedSound() {
  const audioCtx = new (window.AudioContext || window.webkitAudioContext)();

  const playTone = (freq, type, start, duration, volume) => {
    const osc = audioCtx.createOscillator();
    const gain = audioCtx.createGain();

    osc.type = type;
    osc.frequency.setValueAtTime(freq, start);
    osc.frequency.exponentialRampToValueAtTime(freq * 0.5, start + duration);

    gain.gain.setValueAtTime(0, start);
    gain.gain.linearRampToValueAtTime(volume, start + 0.02);
    gain.gain.linearRampToValueAtTime(0, start + duration);

    osc.connect(gain);
    gain.connect(audioCtx.destination);

    osc.start(start);
    osc.stop(start + duration);
  };

  const now = audioCtx.currentTime;

  // Âm thanh "Ting Ting"
  playTone(880, "sine", now, 0.4, 0.3); // A5
  playTone(1320, "sine", now + 0.1, 0.5, 0.2); // E6
}
