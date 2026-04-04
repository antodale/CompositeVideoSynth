const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>COMPOSITE VIDEO SYNTH</title>
  <style>
    :root {
      --bg: #000;
      --fg: #e8e8e8;
      --fg-dim: #555;
      --fg-mid: #999;
      --border: #333;
      --border-active: #e8e8e8;
      --accent: #fff;
    }

    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }

    body {
      background-color: var(--bg);
      color: var(--fg);
      font-family: 'Courier New', Courier, monospace;
      display: flex;
      flex-direction: column;
      align-items: center;
      min-height: 100vh;
      padding: 32px 24px 64px;
    }

    /* ── HEADER ── */
    .header { text-align: center; margin-bottom: 10px; letter-spacing: 0.18em; }
    .title { font-size: clamp(22px, 6vw, 40px); font-weight: 400; line-height: 1.15; text-transform: uppercase; border: 10px solid var(--fg); padding: 18px 28px 14px; }
    .title span { display: block; }
    .byline { font-size: 10px; color: var(--fg-mid); letter-spacing: 0.12em; margin-top: 14px; }

    /* ── NAV BAR ── */
    .nav {
      display: flex;
      gap: 20px;
      margin-top: 24px;
      font-size: 12px;
      letter-spacing: 0.2em;
    }
    .nav a {
      color: var(--fg-dim);
      text-decoration: none;
      transition: color 0.2s;
      cursor: pointer;
    }
    .nav a.active {
      color: var(--fg);
      border-bottom: 1px solid var(--fg);
      padding-bottom: 4px;
    }

    /* ── DIVIDER ── */
    .divider { width: 100%; max-width: 480px; height: 1px; background: var(--border); margin: 28px 0; position: relative; }
    .divider::before { content: '//'; position: absolute; left: 50%; top: 50%; transform: translate(-50%, -50%); background: var(--bg); padding: 0 10px; color: var(--fg-dim); font-size: 10px; letter-spacing: 0.2em; }

    /* ── PANELS (SPA) ── */
    .panel {
      width: 100%;
      max-width: 480px;
      border: 1px solid var(--border);
      padding: 28px;
      display: none; /* Hidden by default */
    }
    .panel.active {
      display: block; /* Shows when active */
    }

    .section-label { font-size: 9px; letter-spacing: 0.3em; color: var(--fg-dim); text-transform: uppercase; margin-bottom: 16px; }

    /* ── EDITOR COMPONENTS ── */
    .radio-group { display: grid; grid-template-columns: repeat(7, 1fr); gap: 6px; margin-bottom: 28px; }
    .radio-group input[type="radio"] { display: none; }
    .radio-group label { display: flex; align-items: center; justify-content: center; border: 1px solid var(--border); color: var(--fg-mid); font-size: 13px; font-family: 'Courier New', monospace; letter-spacing: 0.1em; padding: 10px 0; cursor: pointer; transition: 0.12s; user-select: none; }
    .radio-group input[type="radio"]:checked + label { border-color: var(--border-active); color: var(--fg); background: #111; position: relative; }
    .radio-group input[type="radio"]:checked + label::after { content: ''; position: absolute; bottom: -1px; left: 10%; width: 80%; height: 1px; background: var(--accent); }

    .text-input-area { display: none; margin-bottom: 24px; }
    .text-input-area.visible { display: block; }
    .text-input-area .section-label { margin-bottom: 10px; }
    .text-input-area textarea { width: 100%; background: #080808; border: 1px solid var(--fg-dim); color: var(--fg); padding: 12px; font-family: inherit; font-size: 13px; resize: vertical; outline: none; min-height: 72px; }
    .text-input-area textarea:focus { border-color: var(--fg); }

    .sliders-section .section-label { margin-bottom: 20px; }
    .slider-row { display: flex; align-items: center; gap: 14px; margin-bottom: 18px; }
    .slider-id { font-size: 9px; color: var(--fg-dim); width: 28px; text-align: right; }
    input[type="range"] { -webkit-appearance: none; flex: 1; height: 1px; background: var(--border); outline: none; cursor: pointer; }
    input[type="range"]::-webkit-slider-thumb { -webkit-appearance: none; width: 12px; height: 12px; background: var(--fg); border: none; }
    input[type="range"]::-moz-range-thumb { width: 12px; height: 12px; background: var(--fg); border: none; }
    input[type="range"]::-webkit-slider-runnable-track { --fill-width: calc((100% - 12px) * var(--ratio, 0.5) + 6px); background: linear-gradient(to right, var(--fg-mid) 0%, var(--fg-mid) var(--fill-width), var(--border) var(--fill-width), var(--border) 100%); height: 1px; }
    .slider-value { font-size: 11px; color: var(--fg-mid); width: 34px; text-align: right; }

    /* ── SAVE PRESETS (EDITOR BOTTOM) ── */
    .save-grid { display: grid; grid-template-columns: repeat(6, 1fr); gap: 6px; }
    .save-btn { background: transparent; border: 1px solid var(--border); color: var(--fg-dim); font-family: inherit; font-size: 12px; padding: 10px 0; cursor: pointer; transition: 0.2s; }
    .save-btn:hover { border-color: var(--fg); color: var(--fg); }
    .save-btn:active { background: var(--fg); color: var(--bg); }

    /* ── LIVE MODE COMPONENTS ── */
    .live-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 16px; margin-top: 10px;}
    .live-btn { background: transparent; border: 2px solid var(--border); color: var(--fg-dim); font-family: inherit; font-size: 32px; height: 100px; cursor: pointer; transition: 0.2s; display: flex; align-items: center; justify-content: center; letter-spacing: 0.1em;}
    .live-btn.active { border-color: var(--fg); background: var(--fg); color: var(--bg); font-weight: bold; }

    .footer-tag { font-size: 9px; color: var(--fg-dim); letter-spacing: 0.25em; margin-top: 36px; }
  </style>
</head>
<body>

  <div class="header">
    <div class="title">
      <span>COMPOSITE</span><span>VIDEO</span><span>SYNTH</span>
    </div>
    <div class="byline">a project by reboot_the_rise_of_the_robots</div>
  </div>

  <div class="nav">
    <a id="nav-editor" class="active">[ EDITOR ]</a>
    <span style="color:var(--border)">|</span>
    <a id="nav-live">[ LIVE MODE ]</a>
  </div>

  <div class="divider"></div>

  <div id="panel-editor" class="panel active">
    
    <div class="section-label">// MODE SELECT</div>
    <div class="radio-group" id="modeRadios">
      <input type="radio" name="mode" id="mA" value="A" checked> <label for="mA">A</label>
      <input type="radio" name="mode" id="mB" value="B"> <label for="mB">B</label>
      <input type="radio" name="mode" id="mC" value="C"> <label for="mC">C</label>
      <input type="radio" name="mode" id="mD" value="D"> <label for="mD">D</label>
      <input type="radio" name="mode" id="mE" value="E"> <label for="mE">E</label>
      <input type="radio" name="mode" id="mF" value="F"> <label for="mF">F</label>
      <input type="radio" name="mode" id="mG" value="G"> <label for="mG">G</label>
    </div>

    <div class="text-input-area" id="textInputArea">
      <div class="section-label">// TRANSMISSION DATA</div>
      <textarea placeholder="ENTER TEXT..." maxlength="200" id="transmissionText"></textarea>
    </div>

    <div class="sliders-section">
      <div class="section-label">// PARAMETERS</div>
      <div class="slider-row"><span class="slider-id">P1</span><input type="range" min="0" max="127" value="127" id="s1"><span class="slider-value" id="v1">127</span></div>
      <div class="slider-row"><span class="slider-id">P2</span><input type="range" min="0" max="127" value="64" id="s2"><span class="slider-value" id="v2">064</span></div>
      <div class="slider-row"><span class="slider-id">P3</span><input type="range" min="0" max="127" value="100" id="s3"><span class="slider-value" id="v3">100</span></div>
      <div class="slider-row"><span class="slider-id">P4</span><input type="range" min="0" max="127" value="32" id="s4"><span class="slider-value" id="v4">032</span></div>
      <div class="slider-row"><span class="slider-id">P5</span><input type="range" min="0" max="127" value="90" id="s5"><span class="slider-value" id="v5">090</span></div>
    </div>

    <div class="divider" style="margin: 36px 0 20px;"></div>

    <div class="section-label">// SAVE TO MEMORY BANK</div>
    <div class="save-grid">
      <button class="save-btn" onclick="savePreset(1)">01</button>
      <button class="save-btn" onclick="savePreset(2)">02</button>
      <button class="save-btn" onclick="savePreset(3)">03</button>
      <button class="save-btn" onclick="savePreset(4)">04</button>
      <button class="save-btn" onclick="savePreset(5)">05</button>
      <button class="save-btn" onclick="savePreset(6)">06</button>
    </div>

  </div>

  <div id="panel-live" class="panel">
    <div class="section-label">// SECURE UPLINK TRIGGERS</div>
    <div class="live-grid">
      <button class="live-btn" id="live-1" onclick="loadPreset(1)">01</button>
      <button class="live-btn" id="live-2" onclick="loadPreset(2)">02</button>
      <button class="live-btn" id="live-3" onclick="loadPreset(3)">03</button>
      <button class="live-btn" id="live-4" onclick="loadPreset(4)">04</button>
      <button class="live-btn" id="live-5" onclick="loadPreset(5)">05</button>
      <button class="live-btn" id="live-6" onclick="loadPreset(6)">06</button>
    </div>
  </div>

  <div class="footer-tag">// SIG:CVS-01 ◆ OUTPUT:PAL</div>

  <script>
    let lastSendTime = 0;
    const throttleMs = 50; 

    // --- NAVIGATION LOGIC ---
    const navEditor = document.getElementById('nav-editor');
    const navLive = document.getElementById('nav-live');
    const panelEditor = document.getElementById('panel-editor');
    const panelLive = document.getElementById('panel-live');

    navEditor.addEventListener('click', () => {
      navEditor.classList.add('active');
      navLive.classList.remove('active');
      panelEditor.classList.add('active');
      panelLive.classList.remove('active');
    });

    navLive.addEventListener('click', () => {
      navLive.classList.add('active');
      navEditor.classList.remove('active');
      panelLive.classList.add('active');
      panelEditor.classList.remove('active');
    });

    // --- PAYLOAD BUILDER ---
    function buildPayloadString() {
      const mode = document.querySelector('input[name="mode"]:checked').value;
      let textToSend = "";
      if (mode === 'E' || mode === 'F') {
        textToSend = document.getElementById('transmissionText').value || "";
      }
      const s1 = document.getElementById('s1').value.padStart(3, '0');
      const s2 = document.getElementById('s2').value.padStart(3, '0');
      const s3 = document.getElementById('s3').value.padStart(3, '0');
      const s4 = document.getElementById('s4').value.padStart(3, '0');
      const s5 = document.getElementById('s5').value.padStart(3, '0');

      return `${mode}${textToSend}${s1}${s2}${s3}${s4}${s5}`;
    }

    // --- REALTIME EDITOR SEND ---
    function sendMasterState() {
      const now = Date.now();
      if (now - lastSendTime < throttleMs) return; 
      lastSendTime = now;

      const payload = buildPayloadString();
      // Safely encode the entire string before sending via URL parameters
      fetch(`/updateState?data=${encodeURIComponent(payload)}`)
        .catch(err => console.log("Network error", err));
    }

    // --- PRESET SAVE LOGIC ---
    function savePreset(slot) {
      const payload = buildPayloadString();
      fetch(`/savePreset?slot=${slot}&data=${encodeURIComponent(payload)}`)
        .then(response => {
          if (response.ok) {
            // Visual feedback: Flash the button white briefly
            const btn = document.querySelector(`.save-btn:nth-child(${slot})`);
            const originalText = btn.innerText;
            btn.style.background = 'var(--fg)';
            btn.style.color = 'var(--bg)';
            btn.innerText = 'SAVED';
            setTimeout(() => {
              btn.style.background = 'transparent';
              btn.style.color = 'var(--fg-dim)';
              btn.innerText = originalText;
            }, 500);
          }
        });
    }

    // --- PRESET LOAD (LIVE MODE) LOGIC ---
    function loadPreset(slot) {
      // 1. Tell the ESP32 to load the memory slot and broadcast it
      fetch(`/loadPreset?slot=${slot}`);

      // 2. Update the UI to show which button is active
      document.querySelectorAll('.live-btn').forEach(btn => btn.classList.remove('active'));
      document.getElementById(`live-${slot}`).classList.add('active');
    }

    // --- UI EVENT LISTENERS ---
    const radios = document.querySelectorAll('input[name="mode"]');
    const textArea = document.getElementById('textInputArea');

    function updateVisibility() {
      const selected = document.querySelector('input[name="mode"]:checked').value;
      textArea.classList.toggle('visible', selected === 'E' || selected === 'F');
      sendMasterState(); 
    }
    radios.forEach(r => r.addEventListener('change', updateVisibility));
    updateVisibility(); 

    document.getElementById('transmissionText').addEventListener('input', sendMasterState);

    const sliders = [1, 2, 3, 4, 5];
    sliders.forEach(n => {
      const slider = document.getElementById('s' + n);
      const display = document.getElementById('v' + n);

      function updateVisuals() {
        const val = parseInt(slider.value);
        const min = parseInt(slider.min) || 0;
        const max = parseInt(slider.max) || 127;
        
        display.textContent = String(val).padStart(3, '0');
        const ratio = (val - min) / (max - min);
        slider.style.setProperty('--ratio', ratio);
        
        sendMasterState(); 
      }
      slider.addEventListener('input', updateVisuals);
      updateVisuals(); 
    });
  </script>
</body>
</html>
)rawliteral";