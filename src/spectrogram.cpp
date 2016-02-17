
#include "spectrogram.hpp"
#include <unistd.h>

Spectrogram::Spectrogram(int argc, char *const *argv) : Fft(argc, argv) {
//	if (getenv("FEX_FULLSCREEN"))
		win.create(sf::VideoMode::getDesktopMode(), "Fex", sf::Style::Fullscreen);
//	else
//		win.create(sf::VideoMode(640,480), "Fex");
	aspect = (ntime * win.getSize().y * conf.hop) / (float) (nfreq * win.getSize().x * conf.winlen);
	view.reset(sf::FloatRect(0, -nfreq * (1 + aspect)/2.0, ntime, nfreq * aspect));
	win.setView(view);
	spec.setColor(conf.specFG);
	sf::RenderTexture render_ball;
	render_ball.setSmooth(true);
	render_ball.create(64,64);
	sf::CircleShape dot(32);
	dot.setFillColor(conf.pointBG);
	dot.setOutlineColor(conf.pointFG);
	dot.setOutlineThickness(-8);
	render_ball.draw(dot);
	ball = render_ball.getTexture();
	back.setSize(sf::Vector2f(ntime,-nfreq));
	back.setFillColor(conf.specBG);
	mouse.x = ntime / 2; mouse.y = - nfreq / 2;
	crop1 = sf::Vector2f(-1, -1);
	eraser.setSize(sf::Vector2f(4, 18));
	eraser.setFillColor(sf::Color(255,200,0,200));
	eraser.setPosition(0, 0);
	eraser.setOrigin(2, 10);
}

Spectrogram::~Spectrogram() { }

int Spectrogram::mainLoop() {
	sf::Event ev;
	while (win.isOpen() && win.waitEvent(ev)) {
		evHandler(ev);
		while (win.pollEvent(ev)) evHandler(ev);
		drawMain();
		win.display();
		char out[256];
		snprintf(out,255,"%s - (%0.3fs, %0.3fKHz) FE: %lf ",
			name,
			song.getDuration().asSeconds() * mouse.x / ntime,
			conf.lopass - (conf.hipass - conf.lopass) * mouse.y / nfreq,
			1234.5
		);
		win.setTitle(sf::String(out));
	}
	printf("%lf\t%lf\t%lf\n", pathLength, timeLength, pathLength / timeLength);
	return 0;
}

void Spectrogram::evHandler(sf::Event ev) {
	switch (ev.type) {
		case sf::Event::Closed: evClose(ev); break;
		case sf::Event::KeyPressed: evKeyPress(ev); break;
		case sf::Event::KeyReleased: evKeyRelease(ev); break;
		case sf::Event::MouseMoved: evMouseMove(ev); break;
		case sf::Event::MouseButtonPressed: evMouseButton(ev); break;
		case sf::Event::MouseWheelScrolled: evMouseWheel(ev); break;
		case sf::Event::Resized: evResize(ev); break;
	}
}

void Spectrogram::drawMain() {
	win.clear(conf.winBG);
	win.draw(back);
	win.draw(spec);
	thresh.setColor(conf.threshFG);
	if (toggle.overlay) {
		win.draw(thresh);
		win.draw(getPoints(), &ball);
		win.draw(getLines());
		if (toggle.cursor) {
			if (crop1.x > 0) drawCursor(crop1.x, crop1.y);
			drawCursor(mouse.x, mouse.y);
		}
		if (toggle.eraser)
			win.draw(eraser);
	}
}

void Spectrogram::drawCursor(float x, float y) {
	sf::RectangleShape lineV(sf::Vector2f(1, nfreq));
	lineV.setFillColor(conf.cursorFG);
	lineV.setPosition(view.getViewport().left + x, -nfreq);
	win.draw(lineV);
	sf::RectangleShape lineH(sf::Vector2f(ntime, 1));
	lineH.setFillColor(conf.cursorFG);
	lineH.setPosition(0,view.getViewport().top + y);
	win.draw(lineH);
}

void Spectrogram::listen(float speed) {
	sf::Sound snd(song);
	snd.play();
	snd.setPitch(speed);
	while(snd.getStatus() == 2) {
		sf::RectangleShape line(sf::Vector2f(10, nfreq));
		line.setFillColor(sf::Color(0,255,0,120));
		line.setPosition(ntime * (snd.getPlayingOffset() / song.getDuration()), -nfreq);
		drawMain();
		win.draw(line);
		win.display();
	}
}

void Spectrogram::evClose(sf::Event ev) {
}

void Spectrogram::evKeyPress(sf::Event ev) {
	if (ev.key.code == sf::Keyboard::LShift) toggle.cursor = !toggle.cursor;
	if (ev.key.code == sf::Keyboard::RShift) toggle.cursor = !toggle.cursor;
	if (ev.key.code == sf::Keyboard::LAlt) toggle.eraser = !toggle.eraser;
	if (ev.key.code == sf::Keyboard::RAlt) toggle.eraser = !toggle.eraser;
	if (ev.key.control) switch (ev.key.code) {
		case sf::Keyboard::Space: toggle.overlay = !toggle.overlay; break;
		case sf::Keyboard::Right: conf.floor -= 0.25; makeSpectrogram(); break;
		case sf::Keyboard::Left: conf.floor += 0.25; makeSpectrogram(); break;
		case sf::Keyboard::Up: conf.threshold -= 0.25;  makeOverlay(); break;
		case sf::Keyboard::Down: conf.threshold += 0.25;  makeOverlay(); break;
		case sf::Keyboard::Q: win.close(); break;
		case sf::Keyboard::Num1: listen(1.0); break;
		case sf::Keyboard::Num2: listen(0.5); break;
		case sf::Keyboard::Num3: listen(0.333); break;
		case sf::Keyboard::Num4: listen(0.25); break;
		case sf::Keyboard::Num5: listen(0.2); break;
	}
	else if (ev.key.alt) switch (ev.key.code) {
		case sf::Keyboard::Space: toggle.eraser = !toggle.eraser; break;
	}
	if (ev.key.shift) switch (ev.key.code) {
		case sf::Keyboard::Space: toggle.cursor = !toggle.cursor; break;
	}
}

void Spectrogram::evKeyRelease(sf::Event ev) {
	if (ev.key.code == sf::Keyboard::LShift) toggle.cursor = !toggle.cursor;
	if (ev.key.code == sf::Keyboard::RShift) toggle.cursor = !toggle.cursor;
	if (ev.key.code == sf::Keyboard::LAlt) toggle.eraser = !toggle.eraser;
	if (ev.key.code == sf::Keyboard::RAlt) toggle.eraser = !toggle.eraser;
}

void Spectrogram::evResize(sf::Event ev) {
	aspect = (ntime * win.getSize().y * conf.hop) / (float) (nfreq * win.getSize().x * conf.winlen);
}

void Spectrogram::evMouseMove(sf::Event ev) {
	sf::Vector2f prev = mouse;
	mouse = win.mapPixelToCoords(sf::Vector2i(ev.mouseMove.x,ev.mouseMove.y));
	if (toggle.overlay && toggle.cursor) {
		//
	}
	else if (toggle.overlay && toggle.eraser) {
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && toggle.eraser) erase();
	}
	else {
		if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
			view.setSize(view.getSize().x + prev.x - mouse.x, view.getSize().y + prev.y - mouse.y);
			view.move((prev.x - mouse.x)/2.0, (prev.y - mouse.y)/2.0);
		}
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			view.move(prev.x - mouse.x, prev.y - mouse.y);
		}
		win.setView(view);
	}
	mouse = win.mapPixelToCoords(sf::Vector2i(ev.mouseMove.x,ev.mouseMove.y));
	float in_bounds = 1.0;
	if (mouse.x < 0) in_bounds = mouse.x = 0.0;
	else if (mouse.x > ntime) in_bounds = mouse.x = ntime;
	if (mouse.y > 0) in_bounds = mouse.y = 0.0;
	else if (mouse.y < - nfreq) in_bounds = mouse.y = - nfreq;
	if (in_bounds == 1.0) eraser.setPosition(mouse);
	else eraser.setPosition(sf::Vector2f(-ntime,nfreq));
}

void Spectrogram::erase() {
	sf::FloatRect rect = eraser.getGlobalBounds();
	sf::Transform t = eraser.getInverseTransform();
	sf::Vector2f point;
	int i, j;
	for (i = rect.left - 0.5; i < rect.left + rect.width + 1; ++i) {
		for (j = rect.top - 0.5; j < rect.top + rect.height + 1; ++j) {
			point = t.transformPoint(i + 0.5, j - 0.5);
			if (point.x > -0.25 && point.x < 4.25 && point.y > -0.25 && point.y < 18.25)
				erasePoint(i, -j);
		}
	}
	makeOverlay();
}

void Spectrogram::evMouseButton(sf::Event ev) {
	sf::FloatRect rect;
	if (toggle.overlay && toggle.cursor) switch (ev.mouseButton.button) {
		/* set a crop window: requires 2 presses */
		case sf::Mouse::Button::Left:
			if (crop1.x < 0) crop1 = mouse;
			else { setCrop(crop1, mouse); crop1.x = -1; }
			break;
		/* zoom to fit crop area to window */
		case sf::Mouse::Button::Middle:
			rect = getCrop();
			rect.top -= rect.height * (1 + aspect) / 2.0;
			rect.height *= aspect;
			view.reset(rect);
			win.setView(view);
			break;
		/* reset crop to full song */
		case sf::Mouse::Button::Right:
			crop1.x = -1;
			setCrop(crop1, mouse);
			break;
	}
	else if (toggle.overlay && toggle.eraser) switch (ev.mouseButton.button) {
		case sf::Mouse::Button::Left: if (toggle.eraser) erase(); break;
	}
	else switch (ev.mouseButton.button) {
		/* NOTE: left and right are handled in evMouseMove */
		/* zoom to fit song to window */
		case sf::Mouse::Button::Middle:
			view.reset(sf::FloatRect(0, -nfreq * (1 + aspect)/2.0, ntime, nfreq * aspect));
			win.setView(view);
			break;
	}
}

void Spectrogram::evMouseWheel(sf::Event ev) {
	bool vert = (ev.mouseWheelScroll.wheel == 0);
	float dx = ev.mouseWheelScroll.delta;
	if (toggle.overlay && toggle.cursor) {
		// TODO: anything here?  Probably not.
	}
	if (toggle.overlay && toggle.eraser) {
		/* rotate */
		if (!vert) { eraser.rotate(-dx); return; }
		/* or scale */
		sf::Vector2f scale = eraser.getScale();
		float r = eraser.getRotation();
		if (r < 30 || r > 330 || (r > 150 && r < 210) )
			scale.y += 0.008 * dx;
		else if ( (r > 60 && r < 120) || (r > 240 && r < 300) )
			scale.x += 0.008 * dx;
		else {
			scale.x += 0.004 * dx;
			scale.y += 0.004 * dx;
		}
		if (scale.x < 0.5) scale.x = 0.5;
		else if (scale.x > 5.0) scale.x = 5.0;
		if (scale.y < 0.5) scale.y = 0.5;
		else if (scale.y > 5.0) scale.y = 5.0;
		eraser.setScale(scale);
	}
	/* No modifier wheel movements are for zooming: */
	else if (vert) {
		// TODO: make 0.0075 step size customizable?
		float vx = view.getSize().x / ntime, vy = view.getSize().y / nfreq;
		if (dx < 0 && vx > 1.20 && vy > 1.20) return;
		if (dx > 0 && vx < 0.01 && vy > 0.01) return;
		view.zoom(1.0 - 0.0075 * dx);
		win.setView(view);
	}
}

