
const loadPlayer = ({
  url,
  onDisconnect,
  disconnectThreshold = 3e3,
  ...options
}) => {
    return new Promise((resolve, reject) => {
      // hide the canvas until it's loaded and the correct size
      const originalDisplay = options.canvas.style.display;
      // eslint-disable-next-line no-param-reassign
      //options.canvas.style.display = 'none';

      let lastRx = Date.now(); // Date.now() is more efficient than performance.now()

      if (options.onVideoDecode && onDisconnect) {
        reject(
          new Error('You cannot specify both onDisconnect and onVideoDecode'),
        );
        return;
      }

      const player = new window.JSMpeg.Player(url, {
        // for performance reasons, only record last packet Rx time if onDisconnect is specified
        onVideoDecode: onDisconnect
          ? () => {
              lastRx = Date.now();
            }
          : undefined,
        ...options,
      });

      const o = new MutationObserver((mutations) => {
        if (mutations.some((m) => m.type === 'attributes')) {
          // eslint-disable-next-line no-param-reassign
          options.canvas.style.display = originalDisplay;
          resolve(player);
          o.disconnect();
        }
      });
      o.observe(options.canvas, { attributes: true });

      if (onDisconnect) {
        const i = setInterval(() => {
          if (Date.now() - lastRx > disconnectThreshold) {
            onDisconnect(player);
            clearInterval(i);
          }
        }, disconnectThreshold / 2);
      }
    });
};
  
export { loadPlayer }
