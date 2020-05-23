function setElementProperty(selector, prop, value) {
    const element = document.querySelector(selector);
    
    let obj = element
    const path = prop.split(".")

    while (path.length > 1) obj = obj[path.shift()]
    obj[path[0]] = value;
}

const params = new URLSearchParams(window.location.search)

const log = console.log.bind(console)
const CALL_PREFIX = "CALL:"
const EXECUTE_ORDER_DRAW_THE_SQUARED = "EXECUTE"

let drawRequests = [];

console.log = (message, ...args) => {
    if (message.startsWith(CALL_PREFIX)) {

        const drawRequest = message.slice(CALL_PREFIX.length).replace(" ", "");
        drawRequests.push(drawRequest)
    } else if (message.startsWith(EXECUTE_ORDER_DRAW_THE_SQUARED)) {
        eval(drawRequests.join(";"))
        drawRequests = [];
    } else {
        log(message);
    }
        
}

const keysDown = {};
window.onkeydown = e => keysDown[e.keyCode] = true;
window.onkeyup = e => keysDown[e.keyCode] = false;

const getKeyDown = () => Object.keys(keysDown)
    .find(key => keysDown[key]);

const inputBuffer = [];

window.prompt = () => {
    
    if (!inputBuffer.length) {
        const inputKey = getKeyDown() || 0;
        inputBuffer.push(inputKey);
        inputBuffer.push(null);
    }
    return inputBuffer.shift();
}
