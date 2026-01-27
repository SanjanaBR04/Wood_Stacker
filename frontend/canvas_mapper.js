function drawSheet(canvas, parts) {
    if (!parts || parts.length === 0) {
        console.warn("No parts to draw");
        return;
    }

    const ctx = canvas.getContext('2d');
    const scale = 4;

    ctx.clearRect(0, 0, canvas.width, canvas.height);

    parts.forEach(p => {
        ctx.fillStyle = "#d2b48c";
        ctx.fillRect(p.x * scale, p.y * scale, p.w * scale, p.h * scale);

        ctx.strokeStyle = "#5d4037";
        ctx.lineWidth = 1.5;
        ctx.strokeRect(p.x * scale, p.y * scale, p.w * scale, p.h * scale);

        ctx.fillStyle = "black";
        ctx.textAlign = "center";

        const centerX = (p.x * scale) + (p.w * scale / 2);
        const centerY = (p.y * scale) + (p.h * scale / 2);

        if (p.w * scale > 50 && p.h * scale > 30) {
            ctx.font = "bold 11px Arial";
            ctx.fillText(p.label, centerX, centerY - 5);

            ctx.font = "10px Arial";
            ctx.fillText(p.dims + " cm", centerX, centerY + 10);
        }
    });
}
