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
      padding: 48px 24px 64px;
    }

    /* ── HEADER ── */
    .header {
      text-align: center;
      margin-bottom: 10px;
      letter-spacing: 0.18em;
    }

    .title {
      font-size: clamp(22px, 6vw, 40px);
      font-weight: 400;
      line-height: 1.15;
      text-transform: uppercase;
      border: 10px solid var(--fg);
      padding: 18px 28px 14px;
      position: relative;
    }


    .title span {
      display: block;
    }

    .byline {
      font-size: 10px;
      color: var(--fg-mid);
      letter-spacing: 0.12em;
      text-transform: lowercase;
      margin-top: 14px;
    }

    /* ── DIVIDER ── */
    .divider {
      width: 100%;
      max-width: 480px;
      height: 1px;
      background: var(--border);
      margin: 28px 0;
      position: relative;
    }
    .divider::before {
      content: '//';
      position: absolute;
      left: 50%;
      top: 50%;
      transform: translate(-50%, -50%);
      background: var(--bg);
      padding: 0 10px;
      color: var(--fg-dim);
      font-size: 10px;
      letter-spacing: 0.2em;
    }

    /* ── MAIN PANEL ── */
    .panel {
      width: 100%;
      max-width: 480px;
      border: 1px solid var(--border);
      padding: 28px;
      position: relative;
    }

    .section-label {
      font-size: 9px;
      letter-spacing: 0.3em;
      color: var(--fg-dim);
      text-transform: uppercase;
      margin-bottom: 16px;
    }

    /* ── RADIO GROUP ── */
    .radio-group {
      display: grid;
      grid-template-columns: repeat(7, 1fr);
      gap: 6px;
      margin-bottom: 28px;
    }

    .radio-group input[type="radio"] {
      display: none;
    }

    .radio-group label {
      display: flex;
      align-items: center;
      justify-content: center;
      border: 1px solid var(--border);
      color: var(--fg-mid);
      font-size: 13px;
      font-family: 'Courier New', monospace;
      letter-spacing: 0.1em;
      padding: 10px 0;
      cursor: pointer;
      transition: border-color 0.12s, color 0.12s, background 0.12s;
      user-select: none;
    }

    .radio-group input[type="radio"]:checked + label {
      border-color: var(--border-active);
      color: var(--fg);
      background: #111;
      position: relative;
    }

    .radio-group input[type="radio"]:checked + label::after {
      content: '';
      position: absolute;
      bottom: -1px;
      left: 10%;
      width: 80%;
      height: 1px;
      background: var(--accent);
    }

    .radio-group label:hover {
      border-color: #666;
      color: var(--fg);
    }

    /* ── TEXT INPUT (conditional) ── */
    .text-input-area {
      display: none;
      margin-bottom: 24px;
    }

    .text-input-area.visible {
      display: block;
    }

    .text-input-area .section-label {
      margin-bottom: 10px;
    }

    .text-input-area textarea {
      width: 100%;
      background: #080808;
      border: 1px solid var(--fg-dim);
      color: var(--fg);
      padding: 12px;
      font-family: 'Courier New', monospace;
      font-size: 13px;
      resize: vertical;
      outline: none;
      letter-spacing: 0.05em;
      min-height: 72px;
      line-height: 1.5;
      transition: border-color 0.12s;
    }

    .text-input-area textarea::placeholder {
      color: var(--fg-dim);
      letter-spacing: 0.08em;
    }

    .text-input-area textarea:focus {
      border-color: var(--fg);
    }

    /* ── SLIDERS ── */
    .sliders-section .section-label {
      margin-bottom: 20px;
    }

    .slider-row {
      display: flex;
      align-items: center;
      gap: 14px;
      margin-bottom: 18px;
    }

    .slider-row:last-child {
      margin-bottom: 0;
    }

    .slider-id {
      font-size: 9px;
      letter-spacing: 0.25em;
      color: var(--fg-dim);
      width: 28px;
      flex-shrink: 0;
      text-align: right;
    }

    input[type="range"] {
      -webkit-appearance: none;
      flex: 1;
      height: 1px;
      background: var(--border);
      outline: none;
      cursor: pointer;
      position: relative;
    }

    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 12px;
      height: 12px;
      background: var(--fg);
      border: none;
      cursor: pointer;
      position: relative;
    }

    input[type="range"]::-moz-range-thumb {
      width: 12px;
      height: 12px;
      background: var(--fg);
      border: none;
      cursor: pointer;
    }

    /* Fix: Fill mathematically accounts for the 12px thumb width to align perfectly */
    input[type="range"]::-webkit-slider-runnable-track {
      --fill-width: calc((100% - 12px) * var(--ratio, 0.5) + 6px);
      background: linear-gradient(to right, var(--fg-mid) 0%, var(--fg-mid) var(--fill-width), var(--border) var(--fill-width), var(--border) 100%);
      height: 1px;
    }

    .slider-value {
      font-size: 11px;
      color: var(--fg-mid);
      width: 34px;
      text-align: right;
      flex-shrink: 0;
      letter-spacing: 0.05em;
    }

    /* ── FOOTER TAG ── */
    .footer-tag {
      font-size: 9px;
      color: var(--fg-dim);
      letter-spacing: 0.25em;
      margin-top: 36px;
      text-transform: uppercase;
    }
  </style>
</head>
<body>

  <div class="header">
    <div class="title">
      <span>COMPOSITE</span>
      <span>VIDEO</span>
      <span>SYNTH</span>
    </div>
    <div class="byline">a project by reboot_the_rise_of_the_robots</div>
  </div>

  <div class="divider"></div>

  <div class="panel">

    <!-- MODE SELECT -->
    <div class="section-label">// MODE SELECT</div>
    <div class="radio-group" id="modeRadios">
      <input type="radio" name="mode" id="mA" value="A" checked>
      <label for="mA">A</label>
      <input type="radio" name="mode" id="mB" value="B">
      <label for="mB">B</label>
      <input type="radio" name="mode" id="mC" value="C">
      <label for="mC">C</label>
      <input type="radio" name="mode" id="mD" value="D">
      <label for="mD">D</label>
      <input type="radio" name="mode" id="mE" value="E">
      <label for="mE">E</label>
      <input type="radio" name="mode" id="mF" value="F">
      <label for="mF">F</label>
      <input type="radio" name="mode" id="mG" value="G">
      <label for="mG">G</label>
    </div>

    <!-- TEXT INPUT (E / F only) -->
    <div class="text-input-area" id="textInputArea">
      <div class="section-label">// TRANSMISSION DATA</div>
      <textarea placeholder="ENTER TEXT..." maxlength="200" id="transmissionText"></textarea>
    </div>

    <!-- SLIDERS -->
    <div class="sliders-section">
      <div class="section-label">// PARAMETERS</div>

      <!-- Max set back to 127. Values adjusted to stay under 127 limit -->
      <div class="slider-row">
        <span class="slider-id">P1</span>
        <input type="range" min="0" max="127" value="127" id="s1">
        <span class="slider-value" id="v1">127</span>
      </div>
      <div class="slider-row">
        <span class="slider-id">P2</span>
        <input type="range" min="0" max="127" value="64" id="s2">
        <span class="slider-value" id="v2">064</span>
      </div>
      <div class="slider-row">
        <span class="slider-id">P3</span>
        <input type="range" min="0" max="127" value="100" id="s3">
        <span class="slider-value" id="v3">100</span>
      </div>
      <div class="slider-row">
        <span class="slider-id">P4</span>
        <input type="range" min="0" max="127" value="32" id="s4">
        <span class="slider-value" id="v4">032</span>
      </div>
      <div class="slider-row">
        <span class="slider-id">P5</span>
        <input type="range" min="0" max="127" value="90" id="s5">
        <span class="slider-value" id="v5">090</span>
      </div>
    </div>

  </div>

  <div class="footer-tag">// SIG:CVS-01 ◆ OUTPUT:PAL</div>

<script>
    let lastSendTime = 0;
    const throttleMs = 50; // Sends maximum 20 times per second

// This function gathers ALL data on the page and fires it in one string
    function sendMasterState() {
      const now = Date.now();
      // If 50ms haven't passed since the last send, ignore it!
      if (now - lastSendTime < throttleMs) return; 
      lastSendTime = now;

      // 1. Gather all current values
      const mode = document.querySelector('input[name="mode"]:checked').value;
      
      let textToSend = "";
      // Only attach text data if we are explicitly in Mode E or Mode F
      if (mode === 'E' || mode === 'F') {
        const rawText = document.getElementById('transmissionText').value || "";
        textToSend = encodeURIComponent(rawText);
      }

      // Pad each slider value with leading zeros so it is ALWAYS 3 characters long
      const s1 = document.getElementById('s1').value.padStart(3, '0');
      const s2 = document.getElementById('s2').value.padStart(3, '0');
      const s3 = document.getElementById('s3').value.padStart(3, '0');
      const s4 = document.getElementById('s4').value.padStart(3, '0');
      const s5 = document.getElementById('s5').value.padStart(3, '0');

      // 2. Build the unified packet
      // Example result when no text: A127064100032090
      const payload = `${mode}${textToSend}${s1}${s2}${s3}${s4}${s5}`;
      

      // 3. Shoot it to the ESP32
      fetch(`/updateState?data=${payload}`)
        .catch(err => console.log("Network error", err));
    }

    // --- RADIO BUTTONS ---
    const radios = document.querySelectorAll('input[name="mode"]');
    const textArea = document.getElementById('textInputArea');

    function updateVisibility() {
      const selected = document.querySelector('input[name="mode"]:checked').value;
      textArea.classList.toggle('visible', selected === 'E' || selected === 'F');
      sendMasterState(); // Send on click
    }
    radios.forEach(r => r.addEventListener('change', updateVisibility));
    updateVisibility(); 

    // --- TEXT INPUT ---
    const textBox = document.getElementById('transmissionText');
    // Send state every time a key is pressed! (Throttle protects us here too)
    textBox.addEventListener('input', sendMasterState);

    // --- SLIDERS ---
    const sliders = [1, 2, 3, 4, 5];
    sliders.forEach(n => {
      const slider = document.getElementById('s' + n);
      const display = document.getElementById('v' + n);

      function updateVisuals() {
        const val = parseInt(slider.value);
        const min = parseInt(slider.min) || 0;
        const max = parseInt(slider.max) || 127;
        
        display.textContent = String(val).padStart(3, '0');
        
        // Fix: Use a 0.0 to 1.0 ratio, passing it to CSS calc()
        const ratio = (val - min) / (max - min);
        slider.style.setProperty('--ratio', ratio);
        
        // As long as the slider is moving, keep firing the master state
        sendMasterState(); 
      }

      // 'input' fires continuously while dragging
      slider.addEventListener('input', updateVisuals);
      updateVisuals(); 
    });
  </script>

</body>
</html>
)rawliteral";